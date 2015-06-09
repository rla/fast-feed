#include <node.h>
#include <nan.h>
#include <v8.h>
#include <sstream>
#include <vector>
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

char const *readTextNode(xml_node<char> *node, std::vector<char*> &deallocate) {

    xml_node<char> *textNode = node->first_node();

    if (textNode) {

        // Checks for case of multiple
        // consecutive CDATA nodes.

        if (textNode->next_sibling()) {

            // First calculate required buffer size.

            size_t neededSpace = 0;

            xml_node<char> *sibling = textNode;

            while (sibling) {

                neededSpace += strlen(sibling->value());

                sibling = sibling->next_sibling();
            }

            // Allocate the buffer + space for nul character.

            char* buffer = (char *) malloc(neededSpace + 1);

            // Set nul at end.

            buffer[neededSpace] = '\0';

            sibling = textNode;

            // Offset into buffer.

            size_t offset = 0;

            // Copy to buffer.

            while (sibling) {

                char *value = sibling->value();

                size_t length = strlen(value);

                memcpy(buffer + offset, value, length);

                offset += length;

                sibling = sibling->next_sibling();
            }

            deallocate.push_back(buffer);

            return buffer;

        } else {

            return textNode->value();
        }

    } else {

        return EMPTY_C_STRING;
    }
}

// Same as readTextNode(xml_node<char>, std::vector<char*>) but
// takes an optional child node name.

char const *readTextNode(xml_node<char> *rootNode, const char* name, std::vector<char*> &deallocate) {

    xml_node<char> *node = rootNode->first_node(name);

    if (node) {

        return readTextNode(node, deallocate);

    } else {

        return 0;
    }
}

// Frees all manually allocated strings.

void deallocateStrings(const std::vector<char*> &deallocate) {

    for (std::vector<char*>::const_iterator it = deallocate.begin(); it != deallocate.end(); ++it) {

        free(*it);
    }
}

// Checks whether the node has only
// text, CDATA or comment children.

bool textOnly(xml_node<char> *node) {

    xml_node<char> *child = node->first_node();

    while (child) {

        if (child->type() != node_data &&
            child->type() != node_cdata &&
            child->type() != node_comment) {

            return false;
        }

        child = child->next_sibling();
    }

    return true;
}

// Checks whether the given node is an
// extension node.

bool isExtension(xml_node<char> *node) {

    // Check that name contains
    // the namespace separator.

    return strchr(node->name(), ':') && textOnly(node);
}

// Checks whether the given XML node
// has extensions.

bool hasExtensions(xml_node<char> *node) {

    xml_node<char> *extensionNode = node->first_node();

    while (extensionNode) {

        if (isExtension(extensionNode)) {

            return true;
        }

        extensionNode = extensionNode->next_sibling();
    }

    return false;
}

// Extracts extensions from the given node.
// Assumes that extensions use namespaces.

void doExtractExtensions(xml_node<char> *node, const Local<Object> &base, std::vector<char*> &deallocate) {

    if (hasExtensions(node)) {

        // Only create data structures if there are
        // any extensions.

        Local<Array> extensions = NanNew<Array>();

        int i = 0;

        base->Set(NanNew<String>("extensions"), extensions);

        xml_node<char> *extensionNode = node->first_node();

        while (extensionNode) {

            if (isExtension(extensionNode)) {

                char const *name = extensionNode->name();

                char const *extensionValue = readTextNode(extensionNode, deallocate);

                Local<Object> extension = NanNew<Object>();

                extension->Set(NanNew<String>("name"), NanNew<String>(name));

                extension->Set(NanNew<String>("value"), NanNew<String>(extensionValue));

                xml_attribute<char> *attributeNode = extensionNode->first_attribute();

                if (attributeNode) {

                    Local<Object> attributes = NanNew<Object>();

                    while (attributeNode) {

                        attributes->Set(NanNew<String>(attributeNode->name()), NanNew<String>(attributeNode->value()));

                        attributeNode = attributeNode->next_attribute();
                    }

                    extension->Set(NanNew<String>("attributes"), attributes);
                }

                extensions->Set(NanNew<Number>(i), extension);

                i++;
            }

            extensionNode = extensionNode->next_sibling();
        }
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

// Parses the Atom feed/item author node.

void parseAtomAuthor(xml_node<char> *feedNode, const Local<Object> &base, std::vector<char*> &deallocate) {

    xml_node<char> *authorNode = feedNode->first_node("author");

    if (!authorNode) {

        // No author set.

        return;
    }

    char const *name = readTextNode(authorNode, "name", deallocate);

    if (name) {

        // Name node is set.

        base->Set(NanNew<String>("author"), NanNew<String>(name));

        // Try to get uri and email nodes too.

        char const *uri = readTextNode(authorNode, "uri", deallocate);

        if (uri) {

            base->Set(NanNew<String>("author_uri"), NanNew<String>(uri));
        }

        char const *email = readTextNode(authorNode, "email", deallocate);

        if (email) {

            base->Set(NanNew<String>("author_email"), NanNew<String>(email));
        }

    } else {

        // Whole author node is probably a text node.

        char const *author = readTextNode(feedNode, "author", deallocate);

        if (author) {

            base->Set(NanNew<String>("author"), NanNew<String>(author));
        }
    }
}

// Parses the Atom feed.

void parseAtomFeed(xml_node<char> *feedNode, const Local<Object> &feed, bool extractContent, bool extractExtensions) {

    // Vector string pointers not
    // deallocated by RapidXML.

    std::vector<char*> deallocate;

    feed->Set(NanNew<String>("type"), NanNew<String>("atom"));

    // Extracts the title property.

    char const *title = readTextNode(feedNode, "title", deallocate);

    if (title) {

        feed->Set(NanNew<String>("title"), NanNew<String>(title));
    }

    // Extracts the id property.

    char const *id = readTextNode(feedNode, "id", deallocate);

    if (id) {

        feed->Set(NanNew<String>("id"), NanNew<String>(id));
    }

    // Extracts the link property.

    xml_node<char> *linkNode = feedNode->first_node("link");

    if (linkNode) {

        xml_attribute<char> *hrefAttr = linkNode->first_attribute("href");

        if (hrefAttr) {

            feed->Set(NanNew<String>("link"), NanNew<String>(hrefAttr->value()));
        }
    }

    // Extracts the author property.

    parseAtomAuthor(feedNode, feed, deallocate);

    // Extracts extensions when configured to.

    if (extractExtensions) {

        doExtractExtensions(feedNode, feed, deallocate);
    }

    // Extract all channel items.

    Local<Array> items = NanNew<Array>();

    xml_node<char> *itemNode = feedNode->first_node("entry");

    int i = 0;

    while (itemNode) {

        Local<Object> item = NanNew<Object>();

        // Extracts the id property.

        char const *id = readTextNode(itemNode, "id", deallocate);

        if (id) {

            item->Set(NanNew<String>("id"), NanNew<String>(id));
        }

        Local<Array> links = NanNew<Array>();

        // Extracts all links.
        // 4.2.7. The "atom:link" Element

        xml_node<char> *linkNode = itemNode->first_node("link");

        int linkIndex = 0;

        while (linkNode) {

            Local<Object> link = NanNew<Object>();

            xml_attribute<char> *relAttr = linkNode->first_attribute("rel");

            xml_attribute<char> *hrefAttr = linkNode->first_attribute("href");

            xml_attribute<char> *typeAttr = linkNode->first_attribute("type");

            xml_attribute<char> *hreflangAttr = linkNode->first_attribute("hreflang");

            xml_attribute<char> *titleAttr = linkNode->first_attribute("title");

            xml_attribute<char> *lengthAttr = linkNode->first_attribute("length");

            if (relAttr) {

                link->Set(NanNew<String>("rel"), NanNew<String>(relAttr->value()));
            }

            if (hrefAttr) {

                link->Set(NanNew<String>("href"), NanNew<String>(hrefAttr->value()));
            }

            if (typeAttr) {

                link->Set(NanNew<String>("type"), NanNew<String>(typeAttr->value()));
            }

            if (hreflangAttr) {

                link->Set(NanNew<String>("hreflang"), NanNew<String>(hreflangAttr->value()));
            }

            if (titleAttr) {

                link->Set(NanNew<String>("title"), NanNew<String>(titleAttr->value()));
            }

            if (lengthAttr) {

                link->Set(NanNew<String>("length"), NanNew<String>(lengthAttr->value()));
            }

            xml_node<char> *textNode = linkNode->first_node();

            // This is not by spec but some feeds
            // put URL/IRI into link's text node like:
            // <link>http://example.com</link>

            if (textNode) {

                link->Set(NanNew<String>("text"), NanNew<String>(textNode->value()));
            }

            links->Set(NanNew<Number>(linkIndex), link);

            linkIndex++;

            linkNode = linkNode->next_sibling("link");
        }

        item->Set(NanNew<String>("links"), links);

        // Extract the item title.

        char const *title = readTextNode(itemNode, "title", deallocate);

        if (title) {

            item->Set(NanNew<String>("title"), NanNew<String>(title));
        }

        // Extract the published property.

        char const *date = readTextNode(itemNode, "published", deallocate);

        if (date) {

            item->Set(NanNew<String>("date"), NanNew<String>(date));
        }

        // Extract the updated property.
        // Overwrites date set from published.

        date = readTextNode(itemNode, "updated", deallocate);

        if (date) {

            item->Set(NanNew<String>("date"), NanNew<String>(date));
        }

        // Extract the item author.

        parseAtomAuthor(itemNode, item, deallocate);

        if (extractContent) {

            // Extract the item summary.

            char const *summary = readTextNode(itemNode, "summary", deallocate);

            if (summary) {

                item->Set(NanNew<String>("summary"), NanNew<String>(summary));
            }

            // Extract the item content.

            char const *content = readTextNode(itemNode, "content", deallocate);

            if (content) {

                item->Set(NanNew<String>("content"), NanNew<String>(content));
            }
        }

        // Extracts extensions when configured to.

        if (extractExtensions) {

            doExtractExtensions(itemNode, item, deallocate);
        }

        items->Set(NanNew<Number>(i), item);

        itemNode = itemNode->next_sibling("entry");

        i++;
    }

    feed->Set(NanNew<String>("items"), items);

    // Free created buffers.

    deallocateStrings(deallocate);
}

// Parses the RSS feed.

void parseRssFeed(xml_node<char> *rssNode, const Local<Object> &feed, bool extractContent, bool extractExtensions) {

    // Vector string pointers not
    // deallocated by RapidXML.

    std::vector<char*> deallocate;

    feed->Set(NanNew<String>("type"), NanNew<String>("rss"));

    xml_node<char> *channelNode = rssNode->first_node("channel");

    if (!channelNode) {

        NanThrowTypeError("Invalid RSS channel.");

        return;
    }

    // Extracts the title property.

    char const *title = readTextNode(channelNode, "title", deallocate);

    if (title) {

        feed->Set(NanNew<String>("title"), NanNew<String>(title));
    }

    // Extracts the description property.

    char const *description = readTextNode(channelNode, "description", deallocate);

    if (description) {

        feed->Set(NanNew<String>("description"), NanNew<String>(description));
    }

    // Extracts the link property.

    char const *link = readTextNode(channelNode, "link", deallocate);

    if (link) {

        feed->Set(NanNew<String>("link"), NanNew<String>(link));
    }

    // Extracts the author property.

    char const *author = readTextNode(channelNode, "author", deallocate);

    if (author) {

        feed->Set(NanNew<String>("author"), NanNew<String>(author));
    }

    // Extracts extensions when configured to.

    if (extractExtensions) {

        doExtractExtensions(channelNode, feed, deallocate);
    }

    // Extract all channel items.

    Local<Array> items = NanNew<Array>();

    xml_node<char> *itemNode = channelNode->first_node("item");

    int i = 0;

    while (itemNode) {

        Local<Object> item = NanNew<Object>();

        // Extracts the guid property.

        char const *guid = readTextNode(itemNode, "guid", deallocate);

        if (guid) {

            item->Set(NanNew<String>("id"), NanNew<String>(guid));
        }

        // Extracts the link property.

        char const *link = readTextNode(itemNode, "link", deallocate);

        if (link) {

            item->Set(NanNew<String>("link"), NanNew<String>(link));
        }

        // Extracts the pubDate property.

        char const *date = readTextNode(itemNode, "pubDate", deallocate);

        if (date) {

            item->Set(NanNew<String>("date"), NanNew<String>(date));
        }

        // Sometimes given in Dublin Core extension.

        date = readTextNode(itemNode, "dc:date", deallocate);

        if (date) {

            item->Set(NanNew<String>("date"), NanNew<String>(date));
        }

        // Extract the item title.

        char const *title = readTextNode(itemNode, "title", deallocate);

        if (title) {

            item->Set(NanNew<String>("title"), NanNew<String>(title));
        }

        // Extract the item author.

        char const *author = readTextNode(itemNode, "author", deallocate);

        if (author) {

            item->Set(NanNew<String>("author"), NanNew<String>(author));
        }

        if (extractContent) {

            // Extract the item description.

            char const *description = readTextNode(itemNode, "description", deallocate);

            if (description) {

                item->Set(NanNew<String>("description"), NanNew<String>(description));
            }

            // <content:encoded> is a popular RSS extension.
            // More info: https://developer.mozilla.org/en-US/docs/Web/RSS/Article/Why_RSS_Content_Module_is_Popular_-_Including_HTML_Contents

            char const *content = readTextNode(itemNode, "content:encoded", deallocate);

            if (content) {

                item->Set(NanNew<String>("content"), NanNew<String>(content));
            }
        }

        // Extracts extensions when configured to.

        if (extractExtensions) {

            doExtractExtensions(itemNode, item, deallocate);
        }

        items->Set(NanNew<Number>(i), item);

        itemNode = itemNode->next_sibling("item");

        i++;
    }

    feed->Set(NanNew<String>("items"), items);

    // Free created buffers.

    deallocateStrings(deallocate);
}

NAN_METHOD(ParseFeed) {

    NanScope();

    if (args.Length() < 1) {

        NanThrowTypeError("Wrong number of arguments");

        NanReturnUndefined();
    }

    NanUtf8String xml(args[0]);

    xml_document<char> doc;

    try {

        doc.parse<0>(*xml);

    } catch(rapidxml::parse_error &e) {

        std::pair<int, int> loc = findErrorLine(*xml, e.where<char>());

        char buf[1024];

        snprintf(buf, 1024, "Error on line %d column %d: %s", loc.first, loc.second, e.what());

        NanThrowTypeError(buf);

        NanReturnUndefined();
    }

    bool extractContent = true;

    if (args.Length() >= 2) {

        extractContent = args[1]->BooleanValue();
    }

    bool extractExtensions = false;

    if (args.Length() >= 3) {

        extractExtensions = args[2]->BooleanValue();
    }

    // Creates new object to store the feed
    // contents.

    Local<Object> feed = NanNew<Object>();

    // Tries to get either <rss> or <feed> node.

    xml_node<> *rssNode = doc.first_node("rss");

    if (rssNode) {

        parseRssFeed(rssNode, feed, extractContent, extractExtensions);

    } else {

        xml_node<> *feedNode = doc.first_node("feed");

        if (feedNode) {

            parseAtomFeed(feedNode, feed, extractContent, extractExtensions);

        } else {

            NanThrowTypeError("Invalid feed.");

            NanReturnUndefined();
        }
    }

    NanReturnValue(feed);
}

void init(Handle<Object> exports) {

  exports->Set(NanNew<String>("parse"),
      NanNew<FunctionTemplate>(ParseFeed)->GetFunction());
}

NODE_MODULE(parser, init)
