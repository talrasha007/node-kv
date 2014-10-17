node-kv
=======

一个嵌入式的kv存储引擎，特点就是灰常快。

# 安装
```
npm install node-kv
```

# 用法

### LMDB
```js
var path = require('path'),
    lmdb = require('node-kv').lmdb;
    
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

var txn = env.beginTxn();
db.put(1, 1, txn);
db.get(1, txn);
txn.commit();
```

### LevelDB
即将支持
