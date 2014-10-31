var path = require('path'),
    lmdb = require('../').lmdb;

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
    console.log(cur.gte(0));

    txn.abort();
})();

env.close();
