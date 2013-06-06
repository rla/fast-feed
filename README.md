fast-feed
=========

Node.JS module for parsing newsfeeds (RSS 2 and Atom). It should be one
of the fastest feed parsers. Uses [RapidXML](http://rapidxml.sourceforge.net/).

Usage
-----

Without callback (throws exception):

    var fastFeed = require('fast-feed');
    fastFeed.parse(xml_string);

With callback:

    var fastFeed = require('fast-feed');
    fastFeed.parse(xml_string, function(err, feed) {
        if (err) throw err;
        console.log(feed);
    });

License
-------

The binding code is covered with the MIT license. RapidXML is dual-licensed
(MIT and Boost Software License, see LICENSE.rapidxml).
