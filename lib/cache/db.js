
var Db = module.exports = function (env, opt) {
    this._opt = opt;
    this._env = env;
    env._curEnv.openDb(opt);
    env._oldEnv.openDb(opt);
};

Db.prototype.get = function (key) {
    var env = this._env,
        opt = this._opt,
        curDb = env._curEnv.dbs[opt.name] || env._curEnv.openDb(opt),
        oldDb = env._oldEnv.dbs[opt.name] || env._oldEnv.openDb(opt);

    return curDb.get(key) || oldDb.get(key);
};

Db.prototype.exists = function (key, val) {
    var env = this._env,
        opt = this._opt,
        curDb = env._curEnv.dbs[opt.name] || env._curEnv.openDb(opt),
        oldDb = env._oldEnv.dbs[opt.name] || env._oldEnv.openDb(opt);

    return curDb.exists(key, val) || oldDb.exists(key, val);
};

Db.prototype.put = function (key, val) {
    var env = this._env,
        opt = this._opt,
        curDb = env._curEnv.dbs[opt.name] || env._curEnv.openDb(opt);

    curDb.batchPut(key, val)._cdb = this;
};

Db.prototype.del = function (key) {
    var env = this._env,
        opt = this._opt,
        curDb = env._curEnv.dbs[opt.name] || env._curEnv.openDb(opt),
        oldDb = env._oldEnv.dbs[opt.name] || env._oldEnv.openDb(opt);

    curDb.batchDel(key)._cdb = this;
    oldDb.batchDel(key);
};
