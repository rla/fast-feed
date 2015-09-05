var assert = require('assert');
var parser = require('../');

var rssWithCategories = '<rss version="2.0">' +
    '<channel><item><category>Motosport</category><category>Cars</category>'+
    '</item></channel></rss>';

describe('Parsing of categories', function() {

    it('should find the categories', function () {
      var feed = parser.parse(rssWithCategories);

      assert.equal(feed.items.length, 1);
      assert.equal(
          JSON.stringify(feed.items[0].categories),
          JSON.stringify([
              { name: 'Motosport' },
              { name: 'Cars' }
          ])
      );
    });
});
