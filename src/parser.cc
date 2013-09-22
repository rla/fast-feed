#include <node.h>
#include <v8.h>
#include <sstream>
#include <string.h>
#include "rapidxml.hpp"

// Used example code from:
// https://github.com/glynos/cpp-netlib/blob/master/contrib/http_examples/rss/rss.cpp
// http://nodejs.org/api/addons.html

using namespace v8;
using namespace rapidxml;

char const *EMPTY_C_STRING = "";

// Helper to read text node value.
// Returns 0 when cannot read the value.

char const *readTextNode(xml_node<char> *rootNode, const char* name) {
    xml_node<char> *node = rootNode->first_node(name);
    if (node) {
        xml_node<char> *textNode = node->first_node();
        if (textNode) {
            return textNode->value();
        } else {
            return EMPTY_C_STRING;
        }
    } else {
        return 0;
    }
}

// Helper to find the line number of error.

std::pair<int, int> findErrorLine(const char* xml, const char* where) {
    int i = 0;
    int ln = 1;
    char ch = 0;
    int col = 1;
    while (true) {
        if (xml + i == where) {
            break;
        }
        ch = xml[i];
        if (ch == '\n') {
            ln++;
            col = 0;
        } else {
            col++;
        }
        i++;
    }
    return std::pair<int, int>(ln, col);
}

// Parses the Atom feed.

void parseAtomFeed(xml_node<char> *feedNode, const Local<Object> &feed, bool extractContent) {

    feed->Set(String::NewSymbol("type"), String::New("atom"));

    // Extracts the title property.

    char const *title = readTextNode(feedNode, "title");
    if (title) {
        feed->Set(String::NewSymbol("title"), String::New(title));
    }

    // Extracts the id property.

    char const *id = readTextNode(feedNode, "id");
    if (id) {
        feed->Set(String::NewSymbol("id"), String::New(id));
    }

    // Extracts the link property.
    xml_node<char> *linkNode = feedNode->first_node("link");
    if (linkNode) {
        xml_attribute<char> *hrefAttr = linkNode->first_attribute("href");
        if (hrefAttr) {
            feed->Set(String::NewSymbol("link"), String::New(hrefAttr->value()));
        }
    }

    // Extracts the author property.

    char const *author = readTextNode(feedNode, "author");
    if (author) {
        feed->Set(String::NewSymbol("author"), String::New(author));
    }

    // Extract all channel items.

    Local<Array> items = Array::New();
    xml_node<char> *itemNode = feedNode->first_node("entry");

    int i = 0;
    while (itemNode) {
        Local<Object> item = Object::New();

        // Extracts the id property.

        char const *id = readTextNode(itemNode, "id");
        if (id) {
            item->Set(String::NewSymbol("id"), String::New(id));
        }

        // Extracts the link property.

        xml_node<char> *linkNode = itemNode->first_node("link");

        // Tries to find either <link rel="self"> or <link rel="alternate"> element.
        // https://github.com/rla/fast-feed/issues/2

        bool found = false;
        while (linkNode) {
            xml_attribute<char> *relAttr = linkNode->first_attribute("rel");
            xml_attribute<char> *hrefAttr = linkNode->first_attribute("href");
            if (relAttr && hrefAttr) {
                char const *rel = relAttr->value();
                if (strcmp(rel, "self") == 0 || strcmp(rel, "alternate") == 0) {
                    item->Set(String::NewSymbol("link"), String::New(hrefAttr->value()));
                    found = true;
                    break;
                }
            }
            linkNode = linkNode->next_sibling("link");
        }

        // Did not find rel="self" link.
        // Try take the first link.

        if (!found) {
            xml_node<char> *linkNode = itemNode->first_node("link");
            if (linkNode) {
                xml_attribute<char> *hrefAttr = linkNode->first_attribute("href");
                if (hrefAttr) {
                    item->Set(String::NewSymbol("link"), String::New(hrefAttr->value()));
                }
            }
        }

        // Extract the item title.

        char const *title = readTextNode(itemNode, "title");
        if (title) {
            item->Set(String::NewSymbol("title"), String::New(title));
        }

        // Extract the published property.

        char const *date = readTextNode(itemNode, "published");
        if (date) {
            item->Set(String::NewSymbol("date"), String::New(date));
        }

        // Extract the updated property.
        // Overwrites date set from published.

        date = readTextNode(itemNode, "updated");
        if (date) {
            item->Set(String::NewSymbol("date"), String::New(date));
        }

        // Extract the item author.

        char const *author = readTextNode(itemNode, "author");
        if (author) {
            item->Set(String::NewSymbol("author"), String::New(author));
        }

        if (extractContent) {

            // Extract the item summary.

            char const *summary = readTextNode(itemNode, "summary");
            if (summary) {
                item->Set(String::NewSymbol("summary"), String::New(summary));
            }

            // Extract the item content.

            char const *content = readTextNode(itemNode, "content");
            if (content) {
                item->Set(String::NewSymbol("content"), String::New(content));
            }
        }

        items->Set(Number::New(i), item);
        itemNode = itemNode->next_sibling("entry");
        i++;
    }

    feed->Set(String::NewSymbol("items"), items);
}

// Parses the RSS feed.

void parseRssFeed(xml_node<char> *rssNode, const Local<Object> &feed, bool extractContent) {

    feed->Set(String::NewSymbol("type"), String::New("rss"));

    xml_node<char> *channelNode = rssNode->first_node("channel");
    if (!channelNode) {
        ThrowException(Exception::TypeError(String::New("Invalid RSS channel.")));
        return;
    }

    // Extracts the title property.

    char const *title = readTextNode(channelNode, "title");
    if (title) {
        feed->Set(String::NewSymbol("title"), String::New(title));
    }

    // Extracts the description property.

    char const *description = readTextNode(channelNode, "description");
    if (description) {
        feed->Set(String::NewSymbol("description"), String::New(description));
    }

    // Extracts the link property.

    char const *link = readTextNode(channelNode, "link");
    if (link) {
        feed->Set(String::NewSymbol("link"), String::New(link));
    }

    // Extracts the author property.

    char const *author = readTextNode(channelNode, "author");
    if (author) {
        feed->Set(String::NewSymbol("author"), String::New(author));
    }

    // Extract all channel items.

    Local<Array> items = Array::New();
    xml_node<char> *itemNode = channelNode->first_node("item");

    int i = 0;
    while (itemNode) {
        Local<Object> item = Object::New();

        // Extracts the guid property.

        char const *guid = readTextNode(itemNode, "guid");
        if (guid) {
            item->Set(String::NewSymbol("id"), String::New(guid));
        }

        // Extracts the link property.

        char const *link = readTextNode(itemNode, "link");
        if (link) {
            item->Set(String::NewSymbol("link"), String::New(link));
        }

        // Extracts the pubDate property.

        char const *date = readTextNode(itemNode, "pubDate");
        if (date) {
            item->Set(String::NewSymbol("date"), String::New(date));
        }

        // Sometimes given in Dublin Core extension.

        date = readTextNode(itemNode, "dc:date");
        if (date) {
            item->Set(String::NewSymbol("date"), String::New(date));
        }

        // Extract the item title.

        char const *title = readTextNode(itemNode, "title");
        if (title) {
            item->Set(String::NewSymbol("title"), String::New(title));
        }

        // Extract the item author.

        char const *author = readTextNode(itemNode, "author");
        if (author) {
            item->Set(String::NewSymbol("author"), String::New(author));
        }

        if (extractContent) {

            // Extract the item description.

            char const *description = readTextNode(itemNode, "description");
            if (description) {
                item->Set(String::NewSymbol("description"), String::New(description));
            }
        }

        items->Set(Number::New(i), item);
        itemNode = itemNode->next_sibling("item");
        i++;
    }

    feed->Set(String::NewSymbol("items"), items);
}

Handle<Value> Method(const Arguments& args) {
    HandleScope scope;

    if (args.Length() < 1) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    String::Utf8Value xml(args[0]);

    xml_document<char> doc;

    try {
        doc.parse<0>(*xml);
    } catch(rapidxml::parse_error &e) {
        std::pair<int, int> loc = findErrorLine(*xml, e.where<char>());
        char buf[1024];
        snprintf(buf, 1024, "Error on line %d column %d: %s", loc.first, loc.second, e.what());
        ThrowException(Exception::TypeError(String::New(buf)));
        return scope.Close(Undefined());
    }

    bool extractContent = true;
    if (args.Length() == 2) {
        extractContent = args[1]->BooleanValue();
    }

    // Creates new object to store the feed
    // contents.

    Local<Object> feed = Object::New();

    // Tries to get either <rss> or <feed> node.

    xml_node<> *rssNode = doc.first_node("rss");
    if (rssNode) {
        parseRssFeed(rssNode, feed, extractContent);
    } else {
        xml_node<> *feedNode = doc.first_node("feed");
        if (feedNode) {
            parseAtomFeed(feedNode, feed, extractContent);
        } else {
            ThrowException(Exception::TypeError(String::New("Invalid feed.")));
            return scope.Close(Undefined());
        }
    }

    return scope.Close(feed);
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("parse"),
      FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(parser, init)
