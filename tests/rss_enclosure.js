var assert = require('assert');
var parser = require('../');

var rss = '<rss><channel><title>Test</title>' +
    '<item><title>With enclosure</title><enclosure length="123" url="https://example.com" type="video/wmv"/></item><item></item></channel></rss>';

describe('RSS 2 enclosure support', function() {

    it('should parse the enclosure element', function() {
        var feed = parser.parse(rss);
        assert.equal(feed.items.length, 2);
        var item = feed.items[0];
        assert.equal(item.enclosure.url, 'https://example.com');
        assert.equal(item.enclosure.type, 'video/wmv');
        assert.equal(item.enclosure.length, 123);
    });

    it('has no enclosure property when enclosure is missing', function() {
        var feed = parser.parse(rss);
        assert.equal(feed.items.length, 2);
        var item = feed.items[1];
        assert.equal(typeof item.enclosure, 'undefined');
    });
});
