
var Db = module.exports = function (env, opt) {
    this._opt = opt;
    this._env = env;
    env._curEnv.openDb(opt);
    env._oldEnv.openDb(opt);
};

Db.prototype.get = function (key, txn) {
    var env = this._env,
        opt = this._opt,
        curDb = env._curEnv.dbs[opt.name] || env._curEnv.openDb(opt),
        oldDb = env._oldEnv.dbs[opt.name] || env._oldEnv.openDb(opt);

    return curDb.get(key, txn) || oldDb.get(key);
};

Db.prototype.put = function (key, val, txn) {
    var env = this._env,
        opt = this._opt,
        curDb = env._curEnv.dbs[opt.name] || env._curEnv.openDb(opt);

    try {
        return curDb.put(key, val);
    } catch (e) {
        if (e.message.indexOf('MDB_MAP_FULL') >= 0) {
            this._env._rotate();
            this.put(key, val);
        } else {
            throw (e);
        }
    }
};

Db.prototype.del = function (key, txn) {
    var env = this._env,
        opt = this._opt,
        curDb = env._curEnv.dbs[opt.name] || env._curEnv.openDb(opt);

    return curDb.del(key, txn);
};
