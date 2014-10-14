var path = require('path'),
    lmdb = require('../').lmdb;

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

env.close();
