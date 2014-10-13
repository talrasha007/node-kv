var _ = require('lodash'),
    mkdirp = require('mkdirp'),
    binding = require('../../build/Release/kv.node');

var Db = require('./db.js');

var Env = module.exports = function (opt) {
    var env = this._env = new binding.lmdb.Env();
    env.setMapSize(opt.mapSize || 128 * 1024 * 1024);
    env.setMaxDbs(opt.maxDbs || 32);

    mkdirp.sync(opt.dir);
    env.open(opt.dir);

    this.dir = opt.dir;
    this.dbs = {};
};

Env.prototype.openDb = function (opt) {
    if (!opt.name) throw new Error('LMDB env: openDb requires db name.');
    return this.dbs[opt.name] = new Db(this, opt);
};

Env.prototype.sync = function (cb) {
    if (!cb) throw new Error('LMDB env: sync Must have a callbck to sync LMDB env.');
    this._env.sync(cb);
};

Env.prototype.close = function () {
    console.log('LMDB env: [%s] is closing...', this.dir);

    _.each(this.dbs, function (db) {
        db.close();
    });

    var env = this._env;
    this._env.sync(function () {
        env.close();
    });
};
