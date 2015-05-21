var assert = require('assert');
var parser = require('../');

var cdata_title = '<feed><title><![CDATA[Test]]></title><entry>' +
    '<link>http://example.com/1</link>' +
    '</entry></feed>';

var cdata_multi = '<feed><title><![CDATA[Hello]]> <![CDATA[World]]></title><entry>' +
    '<link>http://example.com/1</link>' +
    '</entry></feed>';

var cdata_mixed = '<feed><title><![CDATA[Hello]]> New <![CDATA[World]]></title><entry>' +
    '<link>http://example.com/1</link>' +
    '</entry></feed>';

describe('CDATA', function() {

    it('should extract CDATA title', function() {
        var feed = parser.parse(cdata_title);
        assert.equal(feed.title, 'Test');
    });

    // NOTE whitespace between consecutive cdata nodes is removed.

    it('should extract CDATA title from multiple CDATA siblings', function() {
        var feed = parser.parse(cdata_multi);
        assert.equal(feed.title, 'HelloWorld');
    });

    it('should extract CDATA title from mixed CDATA siblings', function() {
        var feed = parser.parse(cdata_mixed);
        assert.equal(feed.title, 'Hello New World');
    });
});
