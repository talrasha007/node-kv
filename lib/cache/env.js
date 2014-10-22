var _ = require('lodash'),
    fs = require('fs'),
    path = require('path'),
    mkdirp = require('mkdirp'),
    debug = require('debug')('node-kv'),

    Db = require('./db.js'),
    lmdb = require('../lmdb');

var Env = module.exports = function (opt) {
    if (!opt.dir) throw new Error('Cache Env ctor: dir is required.');

    opt = this._opt = _.defaults(opt, {
        cacheSize: 256 * 1024 * 1024
    });

    opt.mapSize = opt.cacheSize / 2;
    mkdirp.sync(opt.dir);
    this._openEnv();
    this.dbs = {};
};

Env.prototype.flushBatchOps = function () {
    this._oldEnv.flushBatchOps();
    this._curEnv.flushBatchOps();
};

Env.prototype.openDb = function (opt) {
    if (!opt.name) throw new Error('LMDB env: openDb requires db name.');
    if (this.dbs[opt.name]) throw new Error('LMDB env: db[' + opt.name + '] is already opened.');

    return this.dbs[opt.name] = new Db(this, opt);
};

Env.prototype.close = function (cb) {
    if (cb) cb = _.after(2, cb);
    debug('Cache Env: [%s] is closing...', this._opt.dir);
    this._curEnv.close(cb);
    this._oldEnv.close(cb);
    delete this._curEnv;
    delete this._oldEnv;
};

Env.prototype.rmStale = function () {
    var opt = this._opt;
    var dirs = _.chain(fs.readdirSync(opt.dir)).filter(function (d) {
        return parseInt(d) > 0;
    }).sortBy(function (d) { return d * 1; }).value();

    dirs.slice(0, -3).forEach(function (d) {
        require('../rmdir.js')(path.join(opt.dir, d));
    });
};

Env.prototype._rotate = function () {
    debug('Cache Env: [%s] is rotating...', this._opt.dir);
    this._oldEnv.close();
    this._oldEnv = this._curEnv;
    this._curEnv.onfull = function () { throw new Error('This should not happend.'); };
    this._openEnv(true);
};

Env.prototype._openEnv = function (rotate) {
    var opt = this._opt;

    var dirs = _.chain(fs.readdirSync(opt.dir)).filter(function (d) {
        return parseInt(d) > 0;
    }).sortBy(function (d) { return d * 1; }).value();

    if (dirs.length == 0) dirs = ['1', '2'];
    if (dirs.length == 1) dirs.push(parseInt(dirs[dirs.length - 1]) + 1 + '');
    else if (rotate) dirs.push(this._curEnv._opt.num + 1 + '');

    this._curEnv = new lmdb.Env(_.defaults({ num: parseInt(dirs[dirs.length - 1]), dir: path.join(opt.dir, dirs[dirs.length - 1]) }, opt));
    if (!rotate) this._oldEnv = new lmdb.Env(_.defaults({ num: parseInt(dirs[dirs.length - 1]), dir: path.join(opt.dir, dirs[dirs.length - 2]) }, opt));

    var me = this;
    this._curEnv.onfull = function (ops) {
        me._rotate();
        ops.forEach(function (op) {
            op._cdb[op.op](op.key, op.val);
        });
        me._curEnv.flushBatchOps();
    }
};
