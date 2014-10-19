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
    env.flushBatchOps();
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

env.close();
