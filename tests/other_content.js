var assert = require('assert');
var parser = require('../');

var rss = '<rss><channel><title>Test</title>' +
    '<item><title>T1</title></item>ABC<item><title>T2</title></item></channel></rss>';

describe('Parser', function() {

    it('should accept empty title', function() {
        var feed = parser.parse(rss);
        assert.equal(feed.items.length, 2);
    });
});
