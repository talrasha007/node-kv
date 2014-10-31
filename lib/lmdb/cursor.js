var Txn = require('./txn.js'),
    binding = require('../../build/Release/kv.node');

var Cursor = module.exports = function (db, txn) {
    if (!(txn instanceof Txn)) throw new Error('LMDB_cursor: Invalid txn param.');

    var CursorClass = binding.lmdb[['Cursor', db._opt.keyType, db._opt.valueType || db._opt.valType].join('_')];
    if (!CursorClass) throw new Error('Unsupported cursor key/value types, this should not happen.');

    this._keySanitize = db._keySanitize;
    this._valSanitize = db._valSanitize;

    this._cursor = new CursorClass(db._db, txn._txn);
};

function delegate(fn) {
    Cursor.prototype[fn] = function () {
        return this._cursor[fn]();
    }
}

delegate('close');
delegate('del');
delegate('current');
delegate('next');
delegate('prev');
delegate('nextDup');
delegate('prevDup');
delegate('first');
delegate('last');
delegate('firstDup');
delegate('lastDup');

Cursor.prototype.seek = function (key, val) {
    return arguments.length === 1 ?
        this._cursor.seek(this._keySanitize(key)) :
        this._cursor.seek(this._keySanitize(key), this._valSanitize(val));
};

Cursor.prototype.gte = function (key, val) {
    return arguments.length === 1 ?
        this._cursor.gte(this._keySanitize(key)) :
        this._cursor.gte(this._keySanitize(key), this._valSanitize(val));
};
