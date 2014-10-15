var path = require('path'),
    expect = require('expect.js'),
    lmdb = require('../').lmdb;

var env = new lmdb.Env({
    dir: path.join(__dirname, 'testdb')
});

describe('LMDB int32-int32', function () {
    var db = env.openDb({
        name: 'test',
        keyType: 'int32',
        valType: 'int32'
    });

    it('should throw exception when open db with same name.', function () {
        expect(function () {
            env.openDb({ name: 'test' });
        }).to.throwException();
    });

    it('should work as expected.', function () {
        db.put(1, 1);
        expect(db.get(1)).to.be(1);
        db.del(1);
        expect(db.get(1)).to.be(null);
        expect(function () { db.del(1); }).to.throwException();
    });

    it('txn should work as expected.', function () {
        try { db.del(2); } catch (e) { }

        var txn = env.beginTxn();
        db.put(2, 2, txn);
        expect(db.get(2)).to.be(null);
        expect(db.get(2, txn)).to.be(2);
        txn.abort();
        expect(db.get(2)).to.be(null);

        txn = env.beginTxn();
        db.put(2, 2, txn);
        expect(db.get(2)).to.be(null);
        expect(db.get(2, txn)).to.be(2);
        txn.commit();
        expect(db.get(2)).to.be(2);
    });
});