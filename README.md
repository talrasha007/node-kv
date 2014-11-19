#node-kv
An embeded key-value store for node.js, extremely fast.
```
Because rocksdb uses a lot of C++11 features, your compiler should support C++11 to compile it.
I test it on:
  Windows + VS2013
  CentOS 6.5 + gcc 4.8.1
  MacOS
```

[中文文档猛戳此处](https://github.com/talrasha007/node-kv/blob/master/README.zhcn.md)

## Features
- Multiple kv engine support. (LMDB / LevelDB / RocksDB(WIP)).
- High speed. (> 80% speed of c++ version.)
- Compressed bit-vector support, a good choice for bitmap index. (Coming soon.)
- Embeded, easy to use.

## Install
```
npm install node-kv
```

## Test & Benchmark
```
git clone https://github.com/talrasha007/node-kv.git
npm install
npm install -g matcha mocha
mocha   # Run unit test.
matcha  # Run benchmark.
```

## Usage

### - LMDB
It's a lmdb wrapper, for more information about lmdb, [click here](http://symas.com/mdb/) & [documents](http://symas.com/mdb/doc/index.html)
```js
// This example shows how to use lmdb apis.
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
    db.put('Hello', 'world', txn);
    console.log(db.get('Hello', txn));
    txn.abort();
    console.log(db.get('Hello'));
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
// Cache is used for caching hot data, it is a wrapper of lmdb, it holds 2 lmdb envs(current & old), when current env is full,
// it will close old env, set current env as old, and then open a new env as current.
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
