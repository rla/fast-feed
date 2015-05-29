var assert = require('assert');
var parser = require('../');

var rss = '<rss><channel><title>Test</title>' +
    '<item><title>T1</title></item>' +
    '<item><title>T2</title>' +
    '<content:encoded><![CDATA[This is <i>italics</i>.]]></content:encoded>' +
    '</item>' +
    '</channel></rss>';

describe('RSS 2 with content:encoded element', function() {

    it('should parse content:encoded extension', function() {
        var feed = parser.parse(rss);
        assert.equal(feed.items.length, 2);
        assert.equal(feed.items[1].content, 'This is <i>italics</i>.');
    });
});
