node-kv
=======

[中文文档猛戳此处](https://github.com/talrasha007/node-kv/blob/master/README.zhcn.md)

An embeded key-value store for node.js, extremely fast.

## Features
- Multiple kv engine support. (Currently, LMDB only, LevelDB/RocksDB will be supported soon.)
- High speed. LMDB: >1,000,000op/s for get, >100,000op/s for put.
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
env.onfull = function () { console.log('LMDV env is full.'); } // When you use batch ops, remember to set this callback to catch MDB_MAPFULL exception.
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
  - Comming soon.
