var path = require('path'),
    lvldb = require('../').leveldb;

var env = new lvldb.Env({
    dir: path.join(__dirname, 'testdb', 'level'),
    cacheSize: 256 * 1024 * 1024
});

var db = env.openDb({
    name: 'test',
    keyType: 'int32',
    valType: 'int32'
});

db.put(1, 1);
console.log(db.get(1));
db.del(1);
console.log(db.get(1));

db.del(3);
db.batchPut(3, 4);
console.log(db.get(3));
db.flushBatchOps();
console.log(db.get(3));

var cur = db.cursor();
for (var i = cur.first(); i; i = cur.next()) {
    console.log(i);
}
