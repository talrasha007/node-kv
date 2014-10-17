var path = require('path'),
    rmdir = require('../lib/rmdir.js'),
    fs = require('fs'),
    expect = require('expect.js'),
    cache = require('../').cache;

var envPath = path.join(__dirname, 'testdb', 'cache');

describe('Cache', function () {
    it('should create folders as expected.', function (fcb) {
        try { rmdir(envPath); } catch (e) { }

        var ch = new cache.Env({ dir: envPath });
        expect(fs.readdirSync(envPath)).to.eql(['1', '2']);

        ch._rotate();
        expect(fs.readdirSync(envPath)).to.eql(['1', '2', '3']);

        ch._rotate();
        expect(fs.readdirSync(envPath)).to.eql(['1', '2', '3', '4']);

        setTimeout(function () {
            ch.rmStale();
            expect(fs.readdirSync(envPath)).to.eql(['2', '3', '4']);

            ch._rotate();
            ch._rotate();
            expect(fs.readdirSync(envPath)).to.eql(['2', '3', '4', '5', '6']);

            setTimeout(function () {
                ch.rmStale();
                expect(fs.readdirSync(envPath)).to.eql(['4', '5', '6']);
                fcb();
            }, 100);
        }, 100);

    });
});