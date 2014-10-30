var path = require('path'),
    lvl = require('../').leveldb;

var envPath = path.join(__dirname, 'testdb', 'level');
try { require('../lib/rmdir.js')(envPath); } catch (e) { }

var env = new lvl.Env({
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
