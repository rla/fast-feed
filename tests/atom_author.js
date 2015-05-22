var assert = require('assert');
var parser = require('../');

var author_text = '<feed><title>Test</title>' +
    '<author>Author</author><entry>' +
    '<link>http://example.com/1</link>' +
    '</entry></feed>';

var author_name = '<feed><title>Test</title>' +
    '<author><name>Author</name></author><entry>' +
    '<link>http://example.com/1</link>' +
    '</entry></feed>';

var author_full = '<feed><title>Test</title>' +
    '<author><name>Author</name>' +
    '<uri>http://example.com</uri>' +
    '<email>author@example.com</email>' +
    '</author><entry>' +
    '<link>http://example.com/1</link>' +
    '</entry></feed>';

var author_name_cdata = '<feed><title>Test</title>' +
    '<author><name><![CDATA[Author]]></name></author><entry>' +
    '<link>http://example.com/1</link>' +
    '</entry></feed>';

var item_author_text = '<feed><title>Test</title><entry>' +
    '<link>http://example.com/1</link>' +
    '<author>Author</author>' +
    '</entry></feed>';

describe('Atom author', function() {

    it('should parse feed author text node', function() {
        var feed = parser.parse(author_text);
        assert.equal(feed.author, 'Author');
    });

    it('should parse feed author name node', function() {
        var feed = parser.parse(author_name);
        assert.equal(feed.author, 'Author');
    });

    it('should parse feed full author node', function() {
        var feed = parser.parse(author_full);
        assert.equal(feed.author, 'Author');
        assert.equal(feed.author_uri, 'http://example.com');
        assert.equal(feed.author_email, 'author@example.com');
    });

    it('should parse feed author name node containing cdata', function() {
        var feed = parser.parse(author_name_cdata);
        assert.equal(feed.author, 'Author');
    });

    it('should parse item author text node', function() {
        var feed = parser.parse(item_author_text);
        assert.equal(feed.items[0].author, 'Author');
    });
});
