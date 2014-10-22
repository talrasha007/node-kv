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

Db.prototype.exists = function (key, val) {
    return this._db.exists(this._keySanitize(key), this._valSanitize(val));
};

Db.prototype.put = function (key, val, txn) {
    return this._db.put(this._keySanitize(key), this._valSanitize(val), txn && txn._txn);
};

Db.prototype.del = function (key, txn) {
    return this._db.del(this._keySanitize(key), txn && txn._txn);
};

Db.prototype.batchPut = function (key, val, extraData) {
    var me = this;

    key = this._keySanitize(key);
    val = this._valSanitize(val);
    return this._env._pushBatch({
        op: 'put',
        key: key,
        val: val,
        extra: extraData,
        exec: function (txn) {
            me._db.put(key, val, txn._txn);
        }
    });
};

Db.prototype.batchDel = function (key, extraData) {
    var me = this;

    key = this._keySanitize(key);
    return this._env._pushBatch({
        op: 'del',
        key: key,
        extra: extraData,
        exec: function (txn) {
            me._db.del(key, txn._txn);
        }
    });
};
