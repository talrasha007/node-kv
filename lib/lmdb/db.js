var binding = require('../../build/Release/kv.node');

var Db = module.exports = function (env, opt) {
    var dbClass = binding.lmdb[['DB', opt.keyType, opt.valueType || opt.valType].join('_')];
    if (!dbClass) throw new Error('Unsupported db key/value types.');

    this._db = new dbClass(env._env, opt);
};

Db.prototype.close = function () {
    this._db.close();
};
