var binding = require('../../build/Release/kv.node');

var Txn = module.exports = function (env, readonly) {
    var txn = this._txn = new binding.lmdb.Txn(env, readonly);
};

Txn.prototype.commit = function () {
    this._txn.commit();
};

Txn.prototype.abort = function () {
    this._txn.abort();
};

Txn.prototype.reset = function () {
    this._txn.reset();
};

Txn.prototype.renew = function () {
    this._txn.renew();
};
