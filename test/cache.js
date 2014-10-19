var path = require('path'),
    rmdir = require('../lib/rmdir.js'),
    fs = require('fs'),
    expect = require('expect.js'),
    cache = require('../').cache;

var envPath = path.join(__dirname, 'testdb', 'cache');

describe('Cache', function () {
//    it('should create folders as expected.', function (fcb) {
//        try { rmdir(envPath); } catch (e) { }
//
//        var ch = new cache.Env({ dir: envPath });
//        expect(fs.readdirSync(envPath)).to.eql(['1', '2']);
//
//        ch._rotate();
//        expect(fs.readdirSync(envPath)).to.eql(['1', '2', '3']);
//
//        ch._rotate();
//        expect(fs.readdirSync(envPath)).to.eql(['1', '2', '3', '4']);
//
//        setTimeout(function () {
//            ch.rmStale();
//            expect(fs.readdirSync(envPath)).to.eql(['2', '3', '4']);
//
//            ch._rotate();
//            ch._rotate();
//            expect(fs.readdirSync(envPath)).to.eql(['2', '3', '4', '5', '6']);
//
//            setTimeout(function () {
//                ch.rmStale();
//                expect(fs.readdirSync(envPath)).to.eql(['4', '5', '6']);
//
//                ch.close(fcb);
//            }, 100);
//        }, 100);
//    });

    it('should work as expected.', function (fcb) {
        try { rmdir(envPath); } catch (e) { }

        var ch = new cache.Env({ dir: envPath, cacheSize: 1024 * 64 });
        expect(fs.readdirSync(envPath)).to.eql(['1', '2']);

        var cdb = ch.openDb({
            name: 'test',
            keyType: 'int32',
            valType: 'binary'
        });

        var key = 0, val = new Buffer(1024);
        cdb.put(key++, val);
        cdb.put(key++, val);

        ch.flushBatchOps();
        for (var i = 0; i < key; i++) {
            expect(Buffer.isBuffer(cdb.get(i))).to.be(true);
        }
        expect(cdb.get(2)).to.be(null);

        cdb.put(key++, val);
        ch.flushBatchOps();
        expect(Buffer.isBuffer(cdb.get(2))).to.be(true);
        expect(fs.readdirSync(envPath)).to.eql(['1', '2', '3']);

        cdb.put(key++, val);
        ch.flushBatchOps();
        expect(Buffer.isBuffer(cdb.get(0))).to.be(false);
        expect(Buffer.isBuffer(cdb.get(1))).to.be(false);
        expect(Buffer.isBuffer(cdb.get(2))).to.be(true);
        expect(Buffer.isBuffer(cdb.get(3))).to.be(true);
        expect(fs.readdirSync(envPath)).to.eql(['1', '2', '3', '4']);

        ch.rmStale();
        expect(fs.readdirSync(envPath)).to.eql(['2', '3', '4']);

        ch.close(fcb);
    });
});