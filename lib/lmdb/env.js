var _ = require('lodash'),
    mkdirp = require('mkdirp'),
    binding = require('../../build/Release/kv.node');

var Db = require('./db.js'),
    Txn = require('./txn.js');

var Env = module.exports = function (opt) {
    var env = this._env = new binding.lmdb.Env();
    env.setMapSize(opt.mapSize || 128 * 1024 * 1024);
    env.setMaxDbs(opt.maxDbs || 32);

    mkdirp.sync(opt.dir);
    env.open(opt.dir);

    this.dir = opt.dir;
    this.dbs = {};

    this._batchOps = [];
    this._batchSize = parseInt(opt.batchSize) || 64;
};

Env.prototype.openDb = function (opt) {
    if (!opt.name) throw new Error('LMDB env: openDb requires db name.');
    if (this.dbs[opt.name]) throw new Error('LMDB env: db[%s] is already opened.', opt.name);

    return this.dbs[opt.name] = new Db(this, opt);
};

Env.prototype.beginTxn = function (readonly) {
    return new Txn(this._env, !!readonly);
};

Env.prototype._flushBatchOps = function () {
    var txn = this.beginTxn();
    try {
        this._batchOps.forEach(function (op) {
            //op.op.call(op.db, op.key, op.val || txn, op.val && txn);
            op(txn);
        });
        txn.commit();
    } catch (e) {
        txn.abort();
        throw e;
    }

    this._batchOps = [];
};

Env.prototype._pushBatch = function (op) {
    this._batchOps.push(op);
    if (this._batchOps.length >= this._batchSize) this._flushBatchOps();
};

Env.prototype.setBatchSize = function (num) {
    if (!_.isNumber(num)) throw new Error('LMDB env: setBatchSize must have a number param.');

    this._batchSize = Math.min(num, 256);
    if (this._batchOps.length > this._batchSize) this._flushBatchOps();
};

Env.prototype.sync = function (cb) {
    if (!cb) throw new Error('LMDB env: sync Must have a callbck to sync LMDB env.');
    this._env.sync(cb);
};

Env.prototype.close = function (cb) {
    console.log('LMDB env: [%s] is closing...', this.dir);

    _.each(this.dbs, function (db) {
        db.close();
    });

    var env = this._env;
    this._env.sync(function () {
        env.close();
        cb && cb();
    });
};
