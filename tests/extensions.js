var assert = require('assert');
var parser = require('../');

var rss1 = '<rss><channel>' +
    '<item><title>Test</title>' +
    '<link>/test</link>' +
    '<description>Desc</description>' +
    '<dc:creator extra_attribute="value123">Joe Bob Briggs</dc:creator>' +
    '</item></channel></rss>';

// No attributes.

var rss2 = '<rss><channel>' +
    '<item><title>Test</title>' +
    '<link>/test</link>' +
    '<description>Desc</description>' +
    '<dc:creator>Joe Bob Briggs</dc:creator>' +
    '</item></channel></rss>';

// CDATA value

var rss3 = '<rss><channel>' +
    '<item><title>Test</title>' +
    '<link>/test</link>' +
    '<description>Desc</description>' +
    '<dc:creator><![CDATA[Joe Bob Briggs]]></dc:creator>' +
    '</item></channel></rss>';

// Title for channel.

var rss4 = '<rss><channel>' +
    '<dc:title>Test in DC</dc:title>' +
    '<item><title>Test</title>' +
    '<link>/test</link>' +
    '<description>Desc</description>' +
    '<dc:creator><![CDATA[Joe Bob Briggs]]></dc:creator>' +
    '</item></channel></rss>';

// Nested elements.

var rss5 = '<rss><channel>' +
    '<item><title>Test</title>' +
    '<link>/test</link>' +
    '<description>Desc</description>' +
    '<dc:creator><dc:nested>Joe Bob Briggs</dc:nested></dc:creator>' +
    '</item></channel></rss>';

// Duplicate element.

var rss6 = '<rss><channel>' +
    '<item><title>Test</title>' +
    '<link>/test</link>' +
    '<description>Desc</description>' +
    '<dc:creator>Joe Bob Briggs</dc:creator>' +
    '<dc:creator>Joe Bob Briggs dupl</dc:creator>' +
    '</item></channel></rss>';

var atom1 = '<feed><title>Test</title>' +
    '<dc:creator>Joe Bob Briggs</dc:creator>' +
    '<author>Author</author><entry>' +
    '<link>http://example.com/1</link>' +
    '</entry></feed>';

var atom2 = '<feed><title>Test</title>' +
    '<author>Author</author><entry>' +
    '<link>http://example.com/1</link>' +
    '<dc:creator>Joe Bob Briggs</dc:creator>' +
    '</entry></feed>';

describe('Feed extensions', function() {

    it('should parse dc:creator extension', function() {
        var feed = parser.parse(rss1, { extensions: true });
        var extension = feed.items[0].extensions[0];
        assert.equal(extension.name, 'dc:creator');
        assert.equal(extension.value, 'Joe Bob Briggs');
        assert.equal(extension.attributes.extra_attribute, 'value123');
    });

    it('should parse no extensions when not configured to', function() {
        var feed = parser.parse(rss1, { extensions: false });
        var extensions = feed.items[0].extensions;
        assert.equal(typeof extensions, 'undefined');
    });

    it('should parse no extensions by default', function() {
        var feed = parser.parse(rss1, {});
        var extensions = feed.items[0].extensions;
        assert.equal(typeof extensions, 'undefined');
    });

    it('should parse dc:creator extension without attributes', function() {
        var feed = parser.parse(rss2, { extensions: true });
        var extension = feed.items[0].extensions[0];
        assert.equal(extension.value, 'Joe Bob Briggs');
        assert.equal(typeof extension.attributes, 'undefined');
    });

    it('should parse dc:creator extension with CDATA value', function() {
        var feed = parser.parse(rss3, { extensions: true });
        var extension = feed.items[0].extensions[0];
        assert.equal(extension.value, 'Joe Bob Briggs');
    });

    it('should parse extension directly on feed', function() {
        var feed = parser.parse(rss4, { extensions: true });
        var extension = feed.extensions[0];
        assert.equal(extension.name, 'dc:title');
        assert.equal(extension.value, 'Test in DC');
    });

    it('should not parse extensions with nested elements', function() {
        var feed = parser.parse(rss5, { extensions: true });
        var extensions = feed.items[0].extensions;
        assert.equal(typeof extensions, 'undefined');
    });

    it('should parse duplicate dc:creator extension', function() {
        var feed = parser.parse(rss6, { extensions: true });
        var extension1 = feed.items[0].extensions[0];
        assert.equal(extension1.name, 'dc:creator');
        assert.equal(extension1.value, 'Joe Bob Briggs');
        var extension2 = feed.items[0].extensions[1];
        assert.equal(extension2.name, 'dc:creator');
        assert.equal(extension2.value, 'Joe Bob Briggs dupl');
    });

    it('should parse Atom extension', function() {
        var feed = parser.parse(atom1, { extensions: true });
        var extension = feed.extensions[0];
        assert.equal(extension.name, 'dc:creator');
        assert.equal(extension.value, 'Joe Bob Briggs');
    });

    it('should parse Atom item extension', function() {
        var feed = parser.parse(atom2, { extensions: true });
        var extension = feed.items[0].extensions[0];
        assert.equal(extension.name, 'dc:creator');
        assert.equal(extension.value, 'Joe Bob Briggs');
    });
});
