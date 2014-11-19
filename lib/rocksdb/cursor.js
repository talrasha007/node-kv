var binding = require('../../build/Release/kv.node');

var Cursor = module.exports = function (db) {
    var CursorClass = binding.rocksdb[['Cursor', db._opt.keyType, db._opt.valueType || db._opt.valType].join('_')];
    if (!CursorClass) throw new Error('Unsupported cursor key/value types, this should not happen.');

    this._keySanitize = db._keySanitize;
    this._valSanitize = db._valSanitize;

    this._cursor = new CursorClass(db._db);
};

function delegate(fn) {
    Cursor.prototype[fn] = function () {
        return this._cursor[fn]();
    }
}

delegate('key');
delegate('val');

delegate('next');
delegate('prev');
delegate('first');
delegate('last');

Cursor.prototype.gte = function (key) {
    return this._cursor.gte(this._keySanitize(key));
};
