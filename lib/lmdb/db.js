var sanitize = require('../sanitize.js'),
    binding = require('../../build/Release/kv.node');

var Db = module.exports = function (env, opt) {
    var dbClass = binding.lmdb[['DB', opt.keyType, opt.valueType || opt.valType].join('_')];
    if (!dbClass) throw new Error('Unsupported db key/value types.');

    this._db = new dbClass(env._env, opt);
    this._keySanitize = sanitize[opt.keyType];
    this._valSanitize = sanitize[opt.valueType || opt.valType];
};

Db.prototype.close = function () {
    this._db.close();
};

Db.prototype.get = function (key, txn) {
    try {
        return this._db.get(this._keySanitize(key), txn && txn._txn);
    } catch (e) {
        if (e.message.indexOf('MDB_NOTFOUND') >= 0) {
            return null;
        } else {
            throw e;
        }
    }
};

Db.prototype.put = function (key, val, txn) {
    return this._db.put(this._valSanitize(key), this._valSanitize(val), txn && txn._txn);
};

Db.prototype.del = function (key, txn) {
    return this._db.del(this._keySanitize(key), txn && txn._txn);
};
