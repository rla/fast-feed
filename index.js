var native = require('./build/Release/parser');

function parseDate(article) {
    if (typeof article.date !== 'undefined') {
        article.date = new Date(Date.parse(article.date));
    }
}

// Finds "best" link for an Atom feed article.

function atomLink(article) {
    var link;
    var links = article.links;
    var best = false;
    if (links.length > 0) {
        link = links[0];
    }
    for (var i = 0; i < links.length; i++) {
        var l = links[i];
        if (l.rel === 'alternate') {
            if (l.type === 'text/html') {
                link = l;
                best = true;
            } else if (!best) {
                link = l;
            }
        }
    }
    if (link) {
        if (link.href) {
            article.link = link.href;
        } else {
            article.link = link.text;
        }
    }
}

// Postprocess a single Atom feed article.

function postProcAtomArticle(article) {
    parseDate(article);
    atomLink(article);
}

// Postprocess a single RSS 2 feed article.

function postProcRss2Article(article) {
    parseDate(article);
}

// Postprocess the whole Atom feed.

function postProcAtom(feed) {
    feed.items.forEach(postProcAtomArticle);
}

// Postprocess the whole RSS 2 feed.

function postProcRss2(feed) {
    feed.items.forEach(postProcRss2Article);
}

function parseAndPostProc(xml, options) {
    var result = native.parse(xml, options.content, options.extensions);
    if (result.type === 'atom') {
        postProcAtom(result);
    } else {
        postProcRss2(result);
    }
    return result;
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
    if (typeof options.extensions === 'undefined') {
        options.extensions = false;
    }
    var result;
    if (typeof cb === 'function') {
        try {
            result = parseAndPostProc(xml, options);
            cb(null, result);
        } catch (err) {
            cb(err);
        }
    } else {
        result = parseAndPostProc(xml, options);
        return result;
    }
};
