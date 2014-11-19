var _ = require('lodash'),
    path = require('path'),
    sanitize = require('../sanitize.js'),
    binding = require('../../build/Release/kv.node'),

    Cursor = require('./cursor.js'),
    Batch = require('./batch.js');

var Db = module.exports = function (env, opt) {
    var DbClass = binding.rocksdb[['DB', opt.keyType, opt.valueType || opt.valType].join('_')];
    if (!DbClass) throw new Error('Unsupported db key/value types.');

    this._opt = opt;

    this._db = new DbClass(env._env, opt.name);
    this._keySanitize = sanitize[opt.keyType];
    this._valSanitize = sanitize[opt.valueType || opt.valType];

    this._batch = this.createBatch();
    this._batchSize = opt.batchSize || 64;
};

Db.prototype.cursor = function () {
    return new Cursor(this);
};

Db.prototype.setBatchSize = function (num) {
    this._batchSize = num;
};

Db.prototype.createBatch = function () {
    return new Batch(this);
};

Db.prototype.flushBatchOps = function () {
    this._batch.write();
};

Db.prototype.get = function (key) {
    return this._db.get(this._keySanitize(key));
};

Db.prototype.put = function (key, val, batch) {
    return this._db.put(this._keySanitize(key), this._valSanitize(val), batch);
};

Db.prototype.del = function (key, batch) {
    return this._db.del(this._keySanitize(key), batch);
};

Db.prototype.batchPut = function (key, val) {
    this._batch.put(key, val);
    if (this._batch._cnt >= this._batchSize) this.flushBatchOps();
    else this._autoFlush();
};

Db.prototype.batchDel = function (key) {
    this._batch.del(key);
    if (this._batch._cnt >= this._batchSize) this.flushBatchOps();
    else this._autoFlush();
};

Db.prototype._autoFlush = _.debounce(Db.prototype.flushBatchOps, 1);
