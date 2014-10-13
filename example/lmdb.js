var lmdb = require('../').lmdb;

var env = new lmdb.Env({
    dir: '/tmp/testdb',
    mapSize: 8 * 1024 * 1024, // 128M by default
    maxDbs: 64 // 32 by default
});

env.openDb({
    name: 'test',
    keyType: 'int32',
    valType: 'int32' // or valveType
});

env.close();
