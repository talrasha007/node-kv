var fs = require('fs'),
    path = require('path');

var rmdir = module.exports = function (dir) {
    var list = fs.readdirSync(dir);

    list.forEach(function (sub) {
        var filename = path.join(dir, sub),
            stat = fs.statSync(filename);

        if (filename !== '.' || filename !== '..') {
            if (stat.isDirectory()) {
                rmdir(filename);
            } else {
                fs.unlinkSync(filename);
            }
        }

    });

    fs.rmdirSync(dir);
};