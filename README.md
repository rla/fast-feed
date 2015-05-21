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
    items: [{
        id: String,
        title: String,
        date: Date,
        author: String,
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
        link: String
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
    items: [{
        id: String,
        link: String,
        date: Date,
        title: String,
        author: String,
        description: String
    }]
}
```

Any attribute besides `type` might be missing. `description` is missing from items when
the `content: false` option is used.

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

## License

The binding code is covered with the MIT license. RapidXML is dual-licensed
(MIT and Boost Software License, see LICENSE.rapidxml).
