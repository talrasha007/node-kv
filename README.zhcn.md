#node-kv
一个嵌入式的kv存储引擎，特点就是灰常快。
```
由于RocksDB大量使用了C++11特性，所以你需要一个支持C++11特性的编译器来编译。
我已在下列平台编译过：
  Windows + VS2013
  CentOS + gcc 4.8.1
  MacOS
如果你的编译器不支持C++11，你可以选择安装不带RocksDB支持的0.2.x版本。
```

# 特性
- 多个kv引擎的支持(LMDB / LevelDB / RocksDB)
- 高速 (几乎达到C++版本的性能)
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

### - LMDB
关于LMDB, [猛击此处](http://symas.com/mdb/) & [文档](http://symas.com/mdb/doc/index.html)
```js
var path = require('path'),
    lmdb = require('node-kv').lmdb;

var env = new lmdb.Env({
    dir: path.join(__dirname, 'testdb'),
    mapSize: 8 * 1024 * 1024, // 128M by default
    maxDbs: 64 // 32 by default
});

(function () {
    /* Date type can be:
     * string
     * hex - hex string, will convert to binary data equivalent to Buffer(str, 'hex') for storage.
     * int32
     * uint32
     * int64
     * number
     * binary - Buffer object
     */
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

    for (var ok = cur.first(); ok; ok = cur.next()) {
        console.log("Cursor scan: ", cur.key(), cur.val());
    }

    console.log(cur.seek(1));
    console.log(cur.key(), cur.val());
    console.log(cur.gte(0));
    console.log(cur.key(), cur.val());

    txn.abort();
})();

env.close();
```

### - Cache
```js
// This example shows how to use cache apis.
// 这东东是用来做热数据缓存用的，它是一个lmdb的封装，内有两个lmdb env（一个当前，一个旧），
// 当前的满了后，会把旧的env关掉，把当前的设置成旧的，然后新开一个lmdb env做为当前。
var path = require('path'),
    cache = require('node-kv').cache;

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

### - LevelDB
Google [leveldb](https://github.com/google/leveldb) wrapper.
```js
var path = require('path'),
    lvldb = require('node-kv').leveldb;

var env = new lvldb.Env({
    dir: path.join(__dirname, 'testdb', 'level'),
    cacheSize: 256 * 1024 * 1024 // 8MB by default.
});

var db = env.openDb({
    name: 'test',
    keyType: 'int32',
    valType: 'int32'
});

db.put(1, 1);
console.log(db.get(1));
db.del(1);
console.log(db.get(1));

db.del(3);
db.batchPut(3, 4);
console.log(db.get(3));
db.flushBatchOps();
console.log(db.get(3));

var cur = db.cursor();
for (var i = cur.first(); i; i = cur.next()) {
    console.log([cur.key(), cur.val()]);
}
```
### - RocksDB
Facebook [rocksdb](http://rocksdb.org/) wrapper.
```js
var path = require('path'),
    rocksdb = require('node-kv').rocksdb;

var env = new rocksdb.Env({
    dir: path.join(__dirname, 'testdb', 'rocks'),
    cacheSize: 256 * 1024 * 1024 // 4MB by default.
});

var db = env.registerDb({
    name: 'test',
    keyType: 'int32',
    valType: 'int32'
});

env.open();

db.put(1, 1);
console.log(db.get(1));
db.del(1);
console.log(db.get(1));

db.del(3);
db.batchPut(3, 4);
console.log(db.get(3));
db.flushBatchOps();
console.log(db.get(3));

var cur = db.cursor();
for (var i = cur.first(); i; i = cur.next()) {
    console.log([cur.key(), cur.val()]);
}
```
