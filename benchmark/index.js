var path = require('path'),
    lmdb = require('../').lmdb;

var envPath = path.join(__dirname, 'testdb');
try { require('../lib/rmdir.js')(envPath); } catch (e) { }

var env = new lmdb.Env({
    dir: envPath
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

function defineBatch(bsize) {
    suite('LMDB_Batch_seq_' + bsize, function () {
        var i = 0;

        before(function () {
            env.setBatchSize(bsize);
        });

        bench('put', function () { dbI32.batchPut(i++, i); });
        bench('del', function () { dbI32.batchDel(--i); });
    });
}

defineBatch(10);
defineBatch(64);
defineBatch(128);
defineBatch(256);

function defineRandomBatch(bsize) {
    suite('LMDB_Batch_random_' + bsize, function () {
        before(function () {
            env.setBatchSize(bsize);
        });

        bench('put', function () { dbI32.batchPut(Math.random() * 5 * 1024 * 1024, 1); });
    });
}

defineRandomBatch(10);
defineRandomBatch(64);
defineRandomBatch(128);
defineRandomBatch(256);

suite('LMDB_readonly_txn_get', function () {
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
