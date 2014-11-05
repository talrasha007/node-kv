var path = require('path'),
    expect = require('expect.js'),
    lmdb = require('../').lmdb;

var envPath = path.join(__dirname, 'testdb', 'lmdb');
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

    var dupdb = env.openDb({
        name: 'testdup',
        keyType: 'int32',
        valType: 'hex',
        allowDup: true
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

    it('dup should work as expected.', function () {
        dupdb.put(1, '1adb');
        expect(dupdb.exists(1, '1adb')).to.be(true);
        expect(dupdb.exists(1, '2adb')).to.be(false);
        dupdb.put(1, '2adb');
        expect(dupdb.exists(1, '1adb')).to.be(true);
        expect(dupdb.exists(1, '2adb')).to.be(true);

        dupdb.put(1, '2db');
        expect(dupdb.exists(1, '2db0')).to.be(true);

        var txn = env.beginTxn();
        dupdb.put(2, '1adb', txn);
        expect(dupdb.exists(2, '1adb', txn)).to.be(true);
        expect(dupdb.exists(2, '1adb')).to.be(false);
        dupdb.put(2, '2adb', txn);
        expect(dupdb.exists(2, '2adb', txn)).to.be(true);
        expect(dupdb.exists(2, '2adb')).to.be(false);
        txn.commit();
        expect(dupdb.exists(2, '1adb')).to.be(true);
        expect(dupdb.exists(2, '2adb')).to.be(true);
    });

    it('cursor should work as expected.', function () {
        function expectCursor(cur, key, val) {
            expect(cur.key()).to.be(key);
            expect(cur.val()).to.be(val);
        }

        var i, j;
        for (i = 100; i < 110; i++) {
            db.put(i, i);
            for (j = 0; j < 10; j += 3) {
                dupdb.put(i, j + 'abc');
                dupdb.put(i, j + 'abc');
            }
        }

        var txn = env.beginTxn();
        var cur = db.cursor(txn), dupcur = dupdb.cursor(txn);
        expect(function () { cur.key(); }).to.throwException();
        expect(function () { cur.seek(100, 100); }).to.throwException(); // Cannot seek both key/val when allowDup === false.
        for (i = 100; i < 101; i++) {
            expect(cur.seek(i)).to.be(true);
            expectCursor(cur, i, i);

            for (j = 0; j < 10; j++) {
                if (0 === j % 3) {
                    expect(dupcur.seek(i, j + 'abc')).to.be(true);
                    expectCursor(dupcur, i, j + 'abc');
                } else {
                    expect(dupcur.seek(i, j + 'abc')).to.be(false);
                }
            }
        }

        expect(cur.seek(99)).to.be(false);

        expect(cur.gte(99)).to.be(true);
        expectCursor(cur, 100, 100);

        expect(cur.gte(106)).to.be(true);
        expectCursor(cur, 106, 106);

        expect(cur.seek(111)).to.be(false);

        expect(dupcur.gte(99)).to.be(true);
        expectCursor(dupcur, 100, '0abc');

        expect(dupcur.gte(99, '000')).to.be(false);

        expect(dupcur.gte(100, '0abc')).to.be(true);
        expectCursor(dupcur, 100, '0abc');

        expect(dupcur.gte(100, '1abc')).to.be(true);
        expectCursor(dupcur, 100, '3abc');

        expect(dupcur.gte(100, '2abc')).to.be(true);
        expectCursor(dupcur, 100, '3abc');

        expect(dupcur.gte(100, '3abc')).to.be(true);
        expectCursor(dupcur, 100, '3abc');

        expect(dupcur.seek(100)).to.be(true);
        expectCursor(dupcur, 100, '0abc');

        expect(dupcur.nextDup()).to.be(true);
        expectCursor(dupcur, 100, '3abc');

        expect(dupcur.nextDup()).to.be(true);
        expectCursor(dupcur, 100, '6abc');

        expect(dupcur.nextDup()).to.be(true);
        expectCursor(dupcur, 100, '9abc');

        expect(dupcur.nextDup()).to.be(false);

        expect(dupcur.seek(100)).to.be(true);
        expectCursor(dupcur, 100, '0abc');

        expect(dupcur.next()).to.be(true);
        expectCursor(dupcur, 100, '3abc');

        expect(dupcur.next()).to.be(true);
        expectCursor(dupcur, 100, '6abc');

        expect(dupcur.next()).to.be(true);
        expectCursor(dupcur, 100, '9abc');

        expect(dupcur.next()).to.be(true);
        expectCursor(dupcur, 101, '0abc');

        cur.close();
        dupcur.close();
        txn.commit();
    });

    after(function () {
        env.close();
    });
});