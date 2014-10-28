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
cenv.flushBatchOps(); // Data will be flushed automatically after 1ms, if you want to query immediately, do this.
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
    lmdb = require('../').lmdb;

var env = new lmdb.Env({
    dir: path.join(__dirname, 'testdb'),
    mapSize: 8 * 1024 * 1024, // 128M by default
    maxDbs: 64 // 32 by default
});

(function () {
    var db = env.openDb({
        name: 'test',
        keyType: 'int32',
        valType: 'int32' // or valveType
    });

    db.put(1, 1);
    console.log(db.get(1));
    db.del(1);
    console.log(db.get(1));

    db.batchPut(6, 6);
    console.log(db.get(6));
    env.flushBatchOps(); // Data will be flushed automatically after 1ms, if you want to query immediately, do this.
    console.log(db.get(6));
})();

(function () {
    /* Date type can be:
     * string
     * hex - hex string, will convert to a binary data equivalent to Buffer(str, 'hex') for storage.
     * int32
     * uint32
     * int64
     * number
     * binary - Buffer object
     */
    var db = env.openDb({
        name: 'str-test',
        keyType: 'string',
        valType: 'string' // or valveType
    });

    var txn = env.beginTxn();
    db.put('你好', '世界', txn);
    console.log(db.get('你好', txn));
    txn.abort();
    console.log(db.get('你好'));
})();

(function () {
    var db = env.openDb({
        name: 'testdup',
        keyType: 'int32',
        valType: 'int32',
        allowDup: true
    });

    db.put(1, 1);
    console.log(db.exists(1, 1));
    console.log(db.exists(1, 2));
    db.put(1, 2);
    console.log(db.exists(1, 1));
    console.log(db.exists(1, 2));

    // Cursor
    var txn = env.beginTxn(true),
        cur = db.cursor(txn);

    for (var pair = cur.first(); pair; pair = cur.next()) {
        console.log("Cursor scan: ", pair);
    }

    console.log(cur.seek(1));
    console.log(cur.lowerBound(0));

    txn.abort();
})();

env.close();
```

### - LevelDB
即将支持
