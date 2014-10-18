var sanitize = require('../sanitize.js'),
    binding = require('../../build/Release/kv.node');

var Db = module.exports = function (env, opt) {
    var dbClass = binding.lmdb[['DB', opt.keyType, opt.valueType || opt.valType].join('_')];
    if (!dbClass) throw new Error('Unsupported db key/value types.');

    this._env = env;
    this._db = new dbClass(env._env, opt);
    this._keySanitize = sanitize[opt.keyType];
    this._valSanitize = sanitize[opt.valueType || opt.valType];
};

Db.prototype.close = function () {
    this._db.close();
};

Db.prototype.get = function (key, txn) {
    return this._db.get(this._keySanitize(key), txn && txn._txn);
};

Db.prototype.put = function (key, val, txn) {
    return this._db.put(this._keySanitize(key), this._valSanitize(val), txn && txn._txn);
};

Db.prototype.del = function (key, txn) {
    return this._db.del(this._keySanitize(key), txn && txn._txn);
};

Db.prototype.batchPut = function (key, val) {
    var me = this;
    this._env._pushBatch(function (txn) {
        me.put(key, val, txn);
    });
};

Db.prototype.batchDel = function (key) {
    var me = this;
    this._env._pushBatch(function (txn) {
        me.del(key, txn);
    });
};
