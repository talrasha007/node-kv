# LMDB
  It's a lmdb wrapper, for more information about lmdb, [click here](http://symas.com/mdb/) & [documents](http://symas.com/mdb/doc/index.html)

## Usage
```js
var lmdb = require('node-kv').lmdb;
```
  
## Env
### API
  - [ctor](#user-content-ctor)
  - openDb
  - beginTxn
  - flushBatchOps
  - sync
  - close

#### ctor

  A new Env object opens a lmdb env with <a href="http://symas.com/mdb/doc/group__internal.html#ga44d5cd326db2e18f12c59c3eca2c1a3a" target="_blank">MDB_NOSYNC</a> flag.
  We can set the following options to an env:  
  <a href="http://symas.com/mdb/doc/group__internal.html#ga96ac1dd77cc1207915cccb7487d2044f" target="_blank">mapSize</a>  
  <a href="http://symas.com/mdb/doc/group__mdb.html#gaa2fc2f1f37cb1115e733b62cab2fcdbc" target="_blank">maxDbs</a>  
  <a href="#" target="_blank">batchSize</a>  
```js
var env = new (lmdb.Env)({
  dir: '/foo/bar', // Path to env, required, the path will be created is not exists.
  mapSize: 256 * 1024 * 1024, // Bytes, 128MB by default.
  maxDbs: 64, // 32 by default.
  batchSize: 128 // 64 by default.
);

```
