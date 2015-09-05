var assert = require('assert');
var parser = require('../');

var rss = '<rss><channel><title>Test</title>' +
    '<item><title>T1</title></item>ABC<item><title></title></item></channel></rss>';

var rssWithCategories = '<rss version="2.0">' +
    '<channel><item><category>Motosport</category><category>Cars</category>'+
    '</item></channel></rss>';

describe('Parser', function() {

    it('should accept empty title', function() {
        var feed = parser.parse(rss);
        assert.equal(feed.items.length, 2);
    });

    it('should find the categories', function () {
      var feed = parser.parse(rssWithCategories);
      assert.equal(feed.items.length, 1);
      assert.equal(
        JSON.stringify(feed.items[0].categories),
        JSON.stringify([ 'Motosport', 'Cars' ])
      );
    });
});
