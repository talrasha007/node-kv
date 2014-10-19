var path = require('path'),
    expect = require('expect.js'),
    lmdb = require('../').lmdb;

var envPath = path.join(__dirname, 'testdb');
try { require('../lib/rmdir.js')(envPath); } catch (e) { }

describe('LMDB int32-int32', function () {
    var env = new lmdb.Env({
        dir: envPath
    });

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
        expect(db.del(1)).to.be(true);
        expect(db.get(1)).to.be(null);
        expect(db.del(1)).to.be(false);
    });

    it('batch should work as expected.', function (fcb) {
        db.batchPut(6, 6);
        db.batchPut(7, 7);
        db.batchDel(7);
        expect(db.get(6)).to.be(null);
        expect(db.get(7)).to.be(null);
        env.flushBatchOps();
        expect(db.get(6)).to.be(6);
        expect(db.get(7)).to.be(null);
        db.batchPut(7, 7);
        setTimeout(function () {
            expect(db.get(7)).to.be(7);
            fcb();
        }, 50);
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

    after(function () {
        env.close();
    });
});