var binding = require('../../build/Release/kv.node');

var Batch = module.exports = function (db) {
    this._db = db;
    this._batch = new binding.rocksdb.Batch;
    this._cnt = 0;
};

Batch.prototype.del = function (key) {
    this._db.del(key, this._batch);
    this._cnt++;
};

Batch.prototype.put = function (key, val) {
    this._db.put(key, val, this._batch);
    this._cnt++;
};

Batch.prototype.clear = function () {
    this._batch.clear();
    this._cnt = 0;
};

Batch.prototype.flush = Batch.prototype.write = function () {
    if (this._cnt > 0) {
        this._db._db.write(this._batch);
        this.clear();
    }
};
