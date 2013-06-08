var native = require('./build/Release/parser');

function parseDate(article) {
    if (typeof article.date !== 'undefined') {
        article.date = new Date(Date.parse(article.date));
    }
}

exports.parse = function(xml, cb) {
    var result;
    if (typeof cb === 'function') {
        try {
            result = native.parse(xml);
            result.items.forEach(parseDate);
            cb(null, result);
        } catch (err) {
            cb(err);
        }
    } else {
        result = native.parse(xml);
        result.items.forEach(parseDate);
        return result;
    }
};