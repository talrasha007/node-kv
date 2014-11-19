var path = require('path'),
    rocksdb = require('../').rocksdb;

var env = new rocksdb.Env({
    dir: path.join(__dirname, 'testdb', 'rocks'),
    cacheSize: 256 * 1024 * 1024 // 4MB by default.
});

var db = env.registerDb({
    name: 'test',
    keyType: 'int32',
    valType: 'int32'
});

env.open();

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
    console.log([cur.key(), cur.val()]);
}
