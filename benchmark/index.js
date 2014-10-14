var path = require('path'),
    lmdb = require('../').lmdb;

suite('LMDB', function () {
    var env = new lmdb.Env({
        dir: path.join(__dirname, 'testdb')
    });

    var dbI32 = env.openDb({
        name: 'testI32',
        keyType: 'int32',
        valType: 'int32' // or valveType
    });

    bench('int32put', function () {
        dbI32.put(Math.random() * 5000, Math.random() * 5000);
    });

    bench('int32get', function () {
        dbI32.get(Math.random() * 5000);
    });


    var dbString = env.openDb({
        name: 'testString',
        keyType: 'string',
        valType: 'string' // or valveType
    });

    bench('string_put', function () {
        dbI32.put(Math.random() * 5000, Math.random() * 5000);
    });

    bench('string_get', function () {
        dbI32.get(Math.random() * 5000);
    });
});