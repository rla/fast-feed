var native = require('./build/Release/parser');

exports.parse = function(xml, cb) {
    var result;
    if (typeof cb === 'function') {
        try {
            result = native.parse(xml);
            cb(null, result);
        } catch (err) {
            cb(err);
        }
    } else {
        return native.parse(xml);
    }
};