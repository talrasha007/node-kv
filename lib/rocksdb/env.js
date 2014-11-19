var _ = require('lodash'),
    mkdirp = require('mkdirp'),

    binding = require('../../build/Release/kv.node'),
    Db = require('./db.js');

var Env = module.exports = function (opt) {
    mkdirp.sync(opt.dir);
    _.extend(this, opt);

    this._env = new binding.rocksdb.Env();
    this.dbs = {};
};

Env.prototype.registerDb = function (opt) {
    if (!opt.name) throw new Error('LMDB env: openDb requires db name.');
    if (this.dbs[opt.name]) throw new Error('LMDB env: db[' + opt.name + '] is already registered.');

    return this.dbs[opt.name] = new Db(this, opt);
};

Env.prototype.open = function () {
    var me = this;
    this._env.open(this.dir, this.cacheSize);

    _.each(this.dbs, function (db, name) {
        db._db.init(me._env, name);
    });
};
