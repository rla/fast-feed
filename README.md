# fast-feed

Node.JS module for parsing newsfeeds (RSS 2 and Atom). It should be one
of the fastest feed parsers. Uses [RapidXML](http://rapidxml.sourceforge.net/).

## Usage

Without callback (throws exception):

```javascript
var fastFeed = require('fast-feed');
fastFeed.parse(xml_string);
```

With callback:

```javascript
var fastFeed = require('fast-feed');
fastFeed.parse(xml_string, function(err, feed) {
    if (err) throw err;
    console.log(feed);
});
```

Do not parse content/description:

```javascript
var fastFeed = require('fast-feed');
fastFeed.parse(xml_string, { content: false });
```

Or:

```javascript
var fastFeed = require('fast-feed');
fastFeed.parse(xml_string, { content: false }, function(err, feed) {
    if (err) throw err;
    console.log(feed);
});
```

## Extracted feed attributes

For Atom feeds:

```javascript
{
    type: "atom",
    title: String,
    id: String,
    link: String,
    author: String,
    author_uri: String,
    author_email: String,
    extensions: [Extension],
    items: [{
        id: String,
        title: String,
        date: Date,
        author: String,
        author_uri: String,
        author_email: String,
        summary: String,
        content: String,
        links: [{
            rel: String,
            href: String,
            type: String,
            hreflang: String,
            title: String,
            length: String,
            text: String
        }],
        link: String,
        extensions: [Extension]
    }]
}
```

Any attribute besides `type` might be missing. `summary` and `content` are missing when
the `content: false` option is used.

For RSS 2 feeds:

```javascript
{
    type: "rss",
    title: String,
    description: String,
    link: String,
    author: String,
    extensions: [Extension],
    items: [{
        id: String,
        link: String,
        date: Date,
        title: String,
        author: String,
        description: String,
        content: String,
        extensions: [Extension]
    }]
}
```

Any attribute besides `type` might be missing. `description` is missing from items when
the `content: false` option is used.

The `content` property of an RSS 2 item is extracted when the item contains a `<content:encoded>` element.
The information about the content module can be found on [MDN](https://developer.mozilla.org/en-US/docs/Web/RSS/Article/Why_RSS_Content_Module_is_Popular_-_Including_HTML_Contents).

### Feed extensions

Feed extensions are supported on the syntax level. Particulary, any element on the feed/channel/item using a namespace
and having no nested elements is considered an extension. Extensions contain the following data:

```javascript
{
    name: String, // name of the element, such as dc:title
    value: String, // string contents of the element
    attributes: { String: String } // element attributes
}
```

Parsing feed extensions is not enabled by default. Use the `extensions: true` option
for the `parse` function to enable them. The `extensions`/`attributes` property will be missing instead
of being empty when the feed/item has no extension. This has been done to reduce the gc stress.

A single element can have multiple extensions with the same name. Extension names are
not normalized into lowercase.

## Supported Node/io.js versions

This package uses [Nan](https://github.com/rvagg/nan) and has been tested (install+running tests) on:

 * io.js 1.5.1
 * node 0.10.24
 * node 0.12.0

## Running tests

Install dependencies:

    npm install

Build the addon:

    node-gyp configure build

Run tests (requires mocha globally installed):

    make test

## Testing leaks

Leak testing uses assumption that RSS (not the feed but memory) set grows slowly. If it grows
fast then the extension (or something else) leaks.

### Good

RSS grows slowly:

```
$ make test-leak
node --gc_global tests-leak/cdata.js
{ rss: 12419072, heapTotal: 6163968, heapUsed: 2259008 }
{ rss: 12955648, heapTotal: 6163968, heapUsed: 2392424 }
{ rss: 12955648, heapTotal: 6163968, heapUsed: 2257952 }
{ rss: 12955648, heapTotal: 6163968, heapUsed: 2118152 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 1976096 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 3899040 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 3755944 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 3614592 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 3473584 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 3331376 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 3188704 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 3048720 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 2905936 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 2764424 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 2623784 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 2481888 }
{ rss: 14901248, heapTotal: 8261120, heapUsed: 2340056 }
```

### Bad

RSS grows rapidly:

```
$ make test-leak
node --gc_global tests-leak/cdata.js
{ rss: 79446016, heapTotal: 6163968, heapUsed: 2259008 }
{ rss: 146755584, heapTotal: 6163968, heapUsed: 2392424 }
{ rss: 213798912, heapTotal: 6163968, heapUsed: 2257952 }
{ rss: 281112576, heapTotal: 6163968, heapUsed: 2118152 }
{ rss: 349970432, heapTotal: 8261120, heapUsed: 1976096 }
{ rss: 417284096, heapTotal: 8261120, heapUsed: 3899040 }
{ rss: 484597760, heapTotal: 8261120, heapUsed: 3755944 }
{ rss: 551641088, heapTotal: 8261120, heapUsed: 3614592 }
{ rss: 618954752, heapTotal: 8261120, heapUsed: 3473584 }
{ rss: 685998080, heapTotal: 8261120, heapUsed: 3331376 }
{ rss: 753311744, heapTotal: 8261120, heapUsed: 3188704 }
{ rss: 820625408, heapTotal: 8261120, heapUsed: 3048720 }
{ rss: 887668736, heapTotal: 8261120, heapUsed: 2905936 }
{ rss: 954982400, heapTotal: 8261120, heapUsed: 2764424 }
{ rss: 1022025728, heapTotal: 8261120, heapUsed: 2623784 }
{ rss: 1089339392, heapTotal: 8261120, heapUsed: 2481888 }
```

## License

The binding code is covered with the MIT license. RapidXML is dual-licensed
(MIT and Boost Software License, see LICENSE.rapidxml).
