var native = require('./build/Release/parser');

function parseDate(article) {
    if (typeof article.date !== 'undefined') {
        article.date = new Date(Date.parse(article.date));
    }
}

// parse(xml, [options], [cb]).

exports.parse = function(xml, options, cb) {
    // Options not given but callback is.
    if (typeof options === 'function') {
        cb = options;
        options = {};
    }
    // Options not given, callback neither.
    if (typeof options === 'undefined') {
        options = {};
    }
    // Options given, add defaults for
    // non-specified options.
    if (typeof options.content === 'undefined') {
        options.content = true;
    }
    var result;
    if (typeof cb === 'function') {
        try {
            result = native.parse(xml, options.content);
            result.items.forEach(parseDate);
            cb(null, result);
        } catch (err) {
            cb(err);
        }
    } else {
        result = native.parse(xml, options.content);
        result.items.forEach(parseDate);
        return result;
    }
};
