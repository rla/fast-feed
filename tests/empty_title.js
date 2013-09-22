var assert = require('assert');
var parser = require('../');

var rss = '<rss><channel><title></title>' +
    '<item><title></title><link>/test</link><description>Desc</description></item></channel></rss>';

describe('Parser', function() {

    it('should accept empty title', function() {
        var feed = parser.parse(rss);
        assert.equal(feed.title, '');
        assert.equal(feed.items[0].title, '');
    });
});