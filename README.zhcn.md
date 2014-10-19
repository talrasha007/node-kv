node-kv
=======

一个嵌入式的kv存储引擎，特点就是灰常快。

# 特性
- 多个kv引擎的支持(LMDB/LevelDB/RocksDB，那个，现在只有LMDB先。)
- 高速
- 方便
- 即将支持bit-vector运算，会是做bitmap index的一个不错的选择。

# 安装
```
npm install node-kv
```

# Test & Benchmark
```
git clone https://github.com/talrasha007/node-kv.git
npm install
npm install -g matcha mocha
mocha   # Run unit test.
matcha  # Run benchmark.
```

# 用法

### - Cache
```js
// This example shows how to use cache apis.
// 这东东是用来做热数据缓存用的，它是一个lmdb的封装，内有两个lmdb env（一个当前，一个旧），
// 当前的满了后，会把旧的env关掉，把当前的设置成旧的，然后新开一个lmdb env做为当前。
var path = require('path'),
    cache = require('../').cache;

var cenv = new cache.Env({
    dir: path.join(__dirname, 'testdb', 'cache'),
    cacheSize: 128 * 1024 * 1024,   // 256M by default
    batchSize: 128                  // 64 by default
});

var cdb = cenv.openDb({
    name: 'testdb',
    keyType: 'int32',
    valType: 'int32'
});

cdb.put(1, 2);
cdb.put(2, 3);
cenv.flushBatchOps(); // Data will flushed automatically after 1ms, if you want to query immediately, do this.
console.log(cdb.get(1));

cdb.put(3, 3);
setTimeout(function () {
    console.log(cdb.get(3));
    cenv.close();
}, 50);
```

### - LMDB
```js
var path = require('path'),
    lmdb = require('node-kv').lmdb;

var env = new lmdb.Env({
    dir: path.join(__dirname, 'testdb'),
    mapSize: 8 * 1024 * 1024, // 128M by default
    maxDbs: 64 // 32 by default
});

var db = env.openDb({
    name: 'test',
    keyType: 'int32',
    valType: 'int32' // or valveType
});

db.put(1, 1);
console.log(db.get(1));
db.del(1);
console.log(db.get(1));

// Batch
env.onfull = function () { console.log('LMDV env is full.'); } // 使用batch时，记得把这个callback给设上。
db.batchPut(5, 1);
db.batchPut(4, 1);
env.flushBatchOps(); // If you want to query the data immediately, do this.
console.log(db.get(4));

// Txn
var txn = env.beginTxn();
db.put(1, 1, txn);
db.get(1, txn);
txn.commit();
```

### - LevelDB
即将支持
