var path = require('path'),
    expect = require('expect.js'),
    lvl = require('../').leveldb;

var envPath = path.join(__dirname, 'testdb', 'leveldb');
try { require('../lib/rmdir.js')(envPath); } catch (e) { }

describe('LevelDB int32-int32', function () {
    var env = new lvl.Env({
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
        db.del(1);
        expect(db.get(1)).to.be(null);
    });

    it('batchop should work as expected.', function (fcb) {
        db.batchPut(6, 6);
        db.batchPut(7, 7);
        db.batchDel(7);
        expect(db.get(6)).to.be(null);
        expect(db.get(7)).to.be(null);
        db.flushBatchOps();
        expect(db.get(6)).to.be(6);
        expect(db.get(7)).to.be(null);
        db.batchPut(7, 7);
        setTimeout(function () {
            expect(db.get(7)).to.be(7);
            fcb();
        }, 50);
    });

    it('cursor should work as expected.', function () {
        function expectCursor(cur, key, val) {
            expect(cur.key()).to.be(key);
            expect(cur.val()).to.be(val);
        }

        var i, j;
        for (i = 100; i < 110; i++) {
            db.put(i, i);
        }

        var cur = db.cursor();
        expect(cur.key()).to.be(null);
        for (i = 100; i < 101; i++) {
            expect(cur.gte(i)).to.be(true);
            expectCursor(cur, i, i);
        }

        expect(cur.gte(99)).to.be(true);
        expectCursor(cur, 100, 100);

        expect(cur.gte(106)).to.be(true);
        expectCursor(cur, 106, 106);

        expect(cur.gte(111)).to.be(false);
    });
});