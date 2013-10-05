var assert = require('assert');
var parser = require('../');

var atom1 = '<feed><title>Test</title><entry>' +
    '<link rel="replies" type="application/atom+xml" href="http://example.com/1" />' +
    '<link rel="replies" type="text/html" href="http://example.com/2" />' +
    '<link rel="edit" type="application/atom+xml" href="http://example.com/3" />' +
    '<link rel="self" type="application/atom+xml" href="http://example.com/4" />' +
    '<link rel="alternate" type="text/html" href="http://example.com/5" />' +
    '</entry></feed>';

describe('Parser (Atom)', function() {

    it('should parse all entry links', function() {
        var feed = parser.parse(atom1);
        assert.equal(feed.items.length, 1);
        assert.equal(feed.items[0].links.length, 5);
        assert.equal(feed.items[0].link, 'http://example.com/5');
    });

});