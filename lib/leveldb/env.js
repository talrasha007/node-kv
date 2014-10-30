var _ = require('lodash'),
    mkdirp = require('mkdirp'),

    Db = require('./db.js');

var Env = module.exports = function (opt) {
    mkdirp.sync(opt.dir);
    _.extend(this, opt);
    this.dbs = {};
};

Env.prototype.openDb = function (opt) {
    if (!opt.name) throw new Error('LMDB env: openDb requires db name.');
    if (this.dbs[opt.name]) throw new Error('LMDB env: db[' + opt.name + '] is already opened.');

    return this.dbs[opt.name] = new Db(this, opt);
};
