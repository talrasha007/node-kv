var path = require('path'),
    lvl = require('../').leveldb;

var envPath = path.join(__dirname, 'testdb', 'level');
try { require('../lib/rmdir.js')(envPath); } catch (e) { }

var env = new lvl.Env({
    dir: envPath,
    cacheSize: 1024 * 1024 * 32
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

suite('LevelDB', function () {
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

suite('LevelDB cursor', function () {
    var cur;
    before(function () {
        cur = dbI32.cursor();
        cur.first();
    });

    bench('current', function () {
        cur.current();
    });

    bench('seq', function () {
        if (!cur.next()) cur.first();
    });

    bench('random', function () {
        cur.gte(Math.random() * 5000000);
    });
});

function defineSeqBatch(bsize) {
    suite('LevelDB_Batch_seq_' + bsize, function () {
        var i = 0;

        before(function () {
            dbI32.setBatchSize(bsize);
        });

        bench('put', function () { dbI32.batchPut(i++, i); });
        bench('del', function () { dbI32.batchDel(--i); });
    });
}

defineSeqBatch(10);
defineSeqBatch(64);
defineSeqBatch(128);
defineSeqBatch(256);

function defineRandomBatch(bsize) {
    suite('LevelDB_Batch_random_' + bsize, function () {
        var i = 0;

        before(function () {
            dbI32.setBatchSize(bsize);
        });

        bench('put', function () { dbI32.batchPut(Math.random() * 5000000, i++); });
    });
}

defineRandomBatch(10);
defineRandomBatch(64);
defineRandomBatch(128);
defineRandomBatch(256);
