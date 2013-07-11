var assert = require('assert');
var parser = require('../');

var rss = '<rss><channel><title>Test</title>' +
    '<item><link>/test</link><description>Desc</description></item></channel></rss>';

describe('Fast-feed', function() {

    it('should parse both link and description (sync)', function() {
        try {
            var feed = parser.parse(rss);
            assert.equal(feed.items.length, 1);
            assert.equal(feed.items[0].link, '/test');
            assert.equal(feed.items[0].description, 'Desc');
        } catch (err) {
            assert.ifError(err);
        }
    });

    it('should parse both link and description (async)', function() {
        var feed = parser.parse(rss, function(err, feed) {
            assert.ifError(err);
            assert.equal(feed.items.length, 1);
            assert.equal(feed.items[0].link, '/test');
            assert.equal(feed.items[0].description, 'Desc');
        });
    });

    it('should parse both link and description (sync, empty options)', function() {
        try {
            var feed = parser.parse(rss, {});
            assert.equal(feed.items.length, 1);
            assert.equal(feed.items[0].link, '/test');
            assert.equal(feed.items[0].description, 'Desc');
        } catch (err) {
            assert.ifError(err);
        }
    });

    it('should parse both link and description (async, empty options)', function() {
        var feed = parser.parse(rss, {}, function(err, feed) {
            assert.ifError(err);
            assert.equal(feed.items.length, 1);
            assert.equal(feed.items[0].link, '/test');
            assert.equal(feed.items[0].description, 'Desc');
        });
    });

    it('should not parse description (sync)', function() {
        try {
            var feed = parser.parse(rss, { content: false });
            assert.equal(feed.items.length, 1);
            assert.equal(feed.items[0].link, '/test');
            assert.equal(typeof feed.items[0].description, 'undefined');
        } catch (err) {
            assert.ifError(err);
        }
    });

    it('should not parse description (async)', function() {
        var feed = parser.parse(rss, { content: false }, function(err, feed) {
            assert.ifError(err);
            assert.equal(feed.items.length, 1);
            assert.equal(feed.items[0].link, '/test');
            assert.equal(typeof feed.items[0].description, 'undefined');
        });
    });
});
