var util = require('util');
var parser = require('../');

var cdata = '<feed><title><![CDATA[Hello]]> <![CDATA[World]]></title><entry>' +
    '<link>http://example.com/1</link>' +
    '<content><![CDATA[Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc varius mattis convallis. ' +
    'Praesent a massa arcu. Duis nunc erat, tincidunt nec ligula et, lacinia ultricies neque. Morbi at ' +
    'dolor sem. Sed luctus, nunc non ornare viverra, elit leo convallis ex, in lobortis velit turpis vitae ' +
    'dui. Praesent aliquam diam sed erat sodales ullamcorper.]]>' +
    '<![CDATA[Pellentesque pulvinar massa et efficitur porta. ' +
    'Aenean volutpat finibus consectetur. Integer aliquet rutrum dui, ut cursus dui suscipit eu. Nullam eu ' +
    'erat sit amet elit ultrices rhoncus. Aenean sed felis massa. Donec rutrum vehicula leo vitae dapibus. ' +
    'Donec eget placerat lacus.]]></content>' +
    '</entry></feed>';

// Test against possible cdata/multiple text element buffer leak.

for (var i = 0; i < 100; i++) {

    for (var j = 0; j < 100000; j++) {

        parser.parse(cdata);
    }

    console.log(util.inspect(process.memoryUsage()));
}
