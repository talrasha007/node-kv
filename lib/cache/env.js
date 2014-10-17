var _ = require('lodash'),
    fs = require('fs'),
    path = require('path'),
    mkdirp = require('mkdirp'),

    lmdb = require('../lmdb');

var Env = module.exports = function (opt) {
    if (!opt.dir) throw new Error('Cache Env ctor: dir is required.');

    opt = this._opt = _.defaults(opt, {
        cacheSize: 256 * 1024 * 1024
    });

    opt.mapSize = opt.cacheSize / 2;
    mkdirp.sync(opt.dir);
    this._openEnv();
};

Env.prototype.close = function (cb) {
    if (cb) cb = _.after(2, cb);
    console.log('Cache Env [%s] closing...', this._opt.dir);
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
    console.log('Cache Env [%s] rotating...', this._opt.dir);
    this._oldEnv.close();
    this._oldEnv = this._curEnv;
    this._openEnv(true);
};

Env.prototype._openEnv = function (rotate) {
    var opt = this._opt;

    var dirs = _.chain(fs.readdirSync(opt.dir)).filter(function (d) {
        return parseInt(d) > 0;
    }).sortBy(function (d) { return d * 1; }).value();

    if (dirs.length == 0) dirs = ['1', '2'];
    if (rotate || dirs.length == 1) dirs.push(parseInt(dirs[dirs.length - 1]) + 1 + '');

    this._curEnv = new lmdb.Env(_.defaults({ dir: path.join(opt.dir, dirs[dirs.length - 1]) }, opt));
    if (!rotate) this._oldEnv = new lmdb.Env(_.defaults({ dir: path.join(opt.dir, dirs[dirs.length - 2]) }, opt));
};
