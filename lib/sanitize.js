
module.exports = {
    string: function (v) {
        return v;
    },
    number: function (v) {
        v = parseFloat(v);
        if (isNaN(v)) throw new Error('number: Invalid data type.');
        return v;
    },
    int32: function (v) {
        v = parseInt(v);
        if (isNaN(v)) throw new Error('int32: Invalid data type.');
        return v;
    },
    uint32: function (v) {
        v = parseInt(v);
        if (isNaN(v) || v < 0) throw new Error('uint32: Invalid data type.');
        return v;
    },
    int64: function (v) {
        if (!/-?\d+/.test(v)) throw new Error('int64: Invalid data type.');
        return v;
    },
    hex: function (v) {
        if (!/[0-9a-fA-F]+/.test(v)) throw new Error('hex: Invalid data type.');
        return v;
    },
    binary: function (v) {
        if (!Buffer.isBuffer(v)) throw new Error('binary: Invalid data type.');
        return v;
    }
};
