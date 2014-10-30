var path = require('path'),
    sanitize = require('../sanitize.js'),
    binding = require('../../build/Release/kv.node');

var Db = module.exports = function (env, opt) {
    var DbClass = binding.leveldb[['DB', opt.keyType, opt.valueType || opt.valType].join('_')];
    if (!DbClass) throw new Error('Unsupported db key/value types.');

    this._db = new DbClass(path.join(env.dir, opt.name));
    this._keySanitize = sanitize[opt.keyType];
    this._valSanitize = sanitize[opt.valueType || opt.valType];
};

Db.prototype.get = function (key) {
    return this._db.get(this._keySanitize(key));
};

Db.prototype.put = function (key, val) {
    return this._db.put(this._keySanitize(key), this._valSanitize(val));
};

Db.prototype.del = function (key) {
    return this._db.del(this._keySanitize(key));
};
