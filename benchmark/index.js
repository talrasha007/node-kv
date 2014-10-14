var path = require('path'),
    lmdb = require('../').lmdb;

var env = new lmdb.Env({
    dir: path.join(__dirname, 'testdb')
});

var dbI32 = env.openDb({
    name: 'testI32',
    keyType: 'int32',
    valType: 'int32' // or valveType
});

var dbString = env.openDb({
    name: 'testString',
    keyType: 'string',
    valType: 'string' // or valveType
});

suite('LMDB', function () {
    bench('int32put', function () {
        dbI32.put(Math.random() * 5000000, Math.random() * 5000000);
    });

    bench('int32get', function () {
        dbI32.get(Math.random() * 5000000);
    });

    bench('string_put', function () {
        dbString.put(Math.random() * 5000, Math.random() * 5000);
    });

    bench('string_get', function () {
        dbString.get(Math.random() * 5000);
    });
});

suite('LMDB_Batch_put', function () {
    var txn;
    before(function () {
        txn = env.beginTxn();
        txn.cnt = 0;
    });

    function batchPutBench(sz) {
        bench('int32put_batch' + sz, function () {
            dbI32.put(Math.random() * 5000000, Math.random() * 5000000, txn);
            txn.cnt++;
            if (txn.cnt >= sz) {
                txn.cnt = 0;
                txn.commit();
                txn.renew();
            }
        });
    }

    batchPutBench(10);
    batchPutBench(50);
    batchPutBench(100);

    after(function() {
        txn.commit();
    });
});

suite('LMDB_Batch_get', function () {
    var rtxn;

    before(function () {
        rtxn = env.beginTxn(true);
        rtxn.cnt = 0;
    });

    function batchGetBench(sz) {
        bench('int32get_batch' + sz, function () {
            dbI32.get(Math.random() * 5000000, rtxn);
            rtxn.cnt++;
            if (rtxn.cnt >= sz) {
                rtxn.cnt = 0;
                rtxn.reset();
                rtxn.renew();
            }
        });
    }
    batchGetBench(10);
    batchGetBench(50);
    batchGetBench(100);

    after(function() {
        rtxn.abort();
    });
});
