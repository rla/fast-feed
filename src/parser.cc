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
        Local<Array> extensions = Nan::New<Array>();
        int i = 0;
        Nan::Set(base, Nan::New<String>("extensions").ToLocalChecked(), extensions);
        xml_node<char> *extensionNode = node->first_node();
        while (extensionNode) {
            if (isExtension(extensionNode)) {
                char const *name = extensionNode->name();
                char const *extensionValue = readTextNode(extensionNode, deallocate);
                Local<Object> extension = Nan::New<Object>();
                Nan::Set(extension, Nan::New("name").ToLocalChecked(),
                    Nan::New<String>(name).ToLocalChecked());
                Nan::Set(extension, Nan::New<String>("value").ToLocalChecked(),
                    Nan::New<String>(extensionValue).ToLocalChecked());
                xml_attribute<char> *attributeNode = extensionNode->first_attribute();
                if (attributeNode) {
                    Local<Object> attributes = Nan::New<Object>();
                    while (attributeNode) {
                        Nan::Set(attributes, Nan::New<String>(attributeNode->name()).ToLocalChecked(),
                            Nan::New<String>(attributeNode->value()).ToLocalChecked());
                        attributeNode = attributeNode->next_attribute();
                    }
                    Nan::Set(extension, Nan::New<String>("attributes").ToLocalChecked(), attributes);
                }
                Nan::Set(extensions, i, extension);
                i++;
            }
            extensionNode = extensionNode->next_sibling();
        }
    }
}

// Helper to populate object properties from XML
// node attributes.

void populateFromAttributes(xml_node<char> *node, const Local<Object> &target) {
    xml_attribute<char> *attributeNode = node->first_attribute();
    while (attributeNode) {
        Nan::Set(target, Nan::New<String>(attributeNode->name()).ToLocalChecked(),
            Nan::New<String>(attributeNode->value()).ToLocalChecked());
        attributeNode = attributeNode->next_attribute();
    }
}

// Helper to parse string into an integer.
// From: https://stackoverflow.com/questions/14176123/correct-usage-of-strtol
// Returns true when conversion was successful.

bool parseLong(const char *str, long *val) {
    char *temp;
    bool rc = true;
    errno = 0;
    *val = strtol(str, &temp, 0);
    if (temp == str || *temp != '\0' ||
        ((*val == LONG_MIN || *val == LONG_MAX) && errno == ERANGE)) {
            rc = false;
        }
    return rc;
}

// Extracts the enclosure element from the given node.

void doExtractEnclosure(xml_node<char> *node, const Local<Object> &item) {
    xml_node<char> *enclosureNode = node->first_node("enclosure");
    if (!enclosureNode) {
        return;
    }
    Local<Object> enclosure = Nan::New<Object>();
    xml_attribute<char> *lengthAttr = enclosureNode->first_attribute("length");
    if (lengthAttr) {
        long length = 0;
        if (parseLong(lengthAttr->value(), &length)) {
            Nan::Set(enclosure, Nan::New<String>("length").ToLocalChecked(), Nan::New<Number>(length));
        }
    }
    xml_attribute<char> *typeAttr = enclosureNode->first_attribute("type");
    if (typeAttr) {
        Nan::Set(enclosure, Nan::New<String>("type").ToLocalChecked(),
            Nan::New<String>(typeAttr->value()).ToLocalChecked());
    }
    xml_attribute<char> *urlAttr = enclosureNode->first_attribute("url");
    if (urlAttr) {
        Nan::Set(enclosure, Nan::New<String>("url").ToLocalChecked(),
            Nan::New<String>(urlAttr->value()).ToLocalChecked());
    }
    Nan::Set(item, Nan::New<String>("enclosure").ToLocalChecked(), enclosure);
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
        Nan::Set(base, Nan::New<String>("author").ToLocalChecked(),
            Nan::New<String>(name).ToLocalChecked());
        // Try to get uri and email nodes too.
        char const *uri = readTextNode(authorNode, "uri", deallocate);
        if (uri) {
            Nan::Set(base, Nan::New<String>("author_uri").ToLocalChecked(),
                Nan::New<String>(uri).ToLocalChecked());
        }
        char const *email = readTextNode(authorNode, "email", deallocate);
        if (email) {
            Nan::Set(base, Nan::New<String>("author_email").ToLocalChecked(),
                Nan::New<String>(email).ToLocalChecked());
        }
    } else {
        // Whole author node is probably a text node.
        char const *author = readTextNode(feedNode, "author", deallocate);
        if (author) {
            Nan::Set(base, Nan::New<String>("author").ToLocalChecked(),
                Nan::New<String>(author).ToLocalChecked());
        }
    }
}

// Parses the Atom feed.

void parseAtomFeed(xml_node<char> *feedNode, const Local<Object> &feed, bool extractContent, bool extractExtensions) {
    // Vector string pointers not
    // deallocated by RapidXML.
    std::vector<char*> deallocate;
    Nan::Set(feed, Nan::New<String>("type").ToLocalChecked(),
        Nan::New<String>("atom").ToLocalChecked());
    // Extracts the title property.
    char const *title = readTextNode(feedNode, "title", deallocate);
    if (title) {
        Nan::Set(feed, Nan::New<String>("title").ToLocalChecked(),
            Nan::New<String>(title).ToLocalChecked());
    }
    // Extracts the id property.
    char const *id = readTextNode(feedNode, "id", deallocate);
    if (id) {
        Nan::Set(feed, Nan::New<String>("id").ToLocalChecked(),
            Nan::New<String>(id).ToLocalChecked());
    }
    // Extracts the link property.
    xml_node<char> *linkNode = feedNode->first_node("link");
    if (linkNode) {
        xml_attribute<char> *hrefAttr = linkNode->first_attribute("href");
        if (hrefAttr) {
            Nan::Set(feed, Nan::New<String>("link").ToLocalChecked(),
                Nan::New<String>(hrefAttr->value()).ToLocalChecked());
        }
    }
    // Extracts the author property.
    parseAtomAuthor(feedNode, feed, deallocate);
    // Extracts extensions when configured to.
    if (extractExtensions) {
        doExtractExtensions(feedNode, feed, deallocate);
    }
    // Extract all channel items.
    Local<Array> items = Nan::New<Array>();
    xml_node<char> *itemNode = feedNode->first_node("entry");
    int i = 0;
    while (itemNode) {
        Local<Object> item = Nan::New<Object>();
        // Extracts the id property.
        char const *id = readTextNode(itemNode, "id", deallocate);
        if (id) {
            Nan::Set(item, Nan::New<String>("id").ToLocalChecked(),
                Nan::New<String>(id).ToLocalChecked());
        }
        Local<Array> links = Nan::New<Array>();
        // Extracts all links.
        // 4.2.7. The "atom:link" Element
        xml_node<char> *linkNode = itemNode->first_node("link");
        int linkIndex = 0;
        while (linkNode) {
            Local<Object> link = Nan::New<Object>();
            xml_attribute<char> *relAttr = linkNode->first_attribute("rel");
            xml_attribute<char> *hrefAttr = linkNode->first_attribute("href");
            xml_attribute<char> *typeAttr = linkNode->first_attribute("type");
            xml_attribute<char> *hreflangAttr = linkNode->first_attribute("hreflang");
            xml_attribute<char> *titleAttr = linkNode->first_attribute("title");
            xml_attribute<char> *lengthAttr = linkNode->first_attribute("length");
            if (relAttr) {
                link->Set(Nan::New<String>("rel").ToLocalChecked(),
                    Nan::New<String>(relAttr->value()).ToLocalChecked());
            }
            if (hrefAttr) {
                link->Set(Nan::New<String>("href").ToLocalChecked(),
                    Nan::New<String>(hrefAttr->value()).ToLocalChecked());
            }
            if (typeAttr) {
                link->Set(Nan::New<String>("type").ToLocalChecked(),
                    Nan::New<String>(typeAttr->value()).ToLocalChecked());
            }
            if (hreflangAttr) {
                link->Set(Nan::New<String>("hreflang").ToLocalChecked(),
                    Nan::New<String>(hreflangAttr->value()).ToLocalChecked());
            }
            if (titleAttr) {
                link->Set(Nan::New<String>("title").ToLocalChecked(),
                    Nan::New<String>(titleAttr->value()).ToLocalChecked());
            }
            if (lengthAttr) {
                link->Set(Nan::New<String>("length").ToLocalChecked(),
                    Nan::New<String>(lengthAttr->value()).ToLocalChecked());
            }
            xml_node<char> *textNode = linkNode->first_node();
            // This is not by spec but some feeds
            // put URL/IRI into link's text node like:
            // <link>http://example.com</link>
            if (textNode) {
                link->Set(Nan::New<String>("text").ToLocalChecked(),
                    Nan::New<String>(textNode->value()).ToLocalChecked());
            }
            links->Set(Nan::New<Number>(linkIndex), link);
            linkIndex++;
            linkNode = linkNode->next_sibling("link");
        }
        Nan::Set(item, Nan::New<String>("links").ToLocalChecked(), links);
        // Extract the item title.
        char const *title = readTextNode(itemNode, "title", deallocate);
        if (title) {
            Nan::Set(item, Nan::New<String>("title").ToLocalChecked(),
                Nan::New<String>(title).ToLocalChecked());
        }
        // Extract the published property.
        char const *date = readTextNode(itemNode, "published", deallocate);
        if (date) {
            Nan::Set(item, Nan::New<String>("date").ToLocalChecked(),
                Nan::New<String>(date).ToLocalChecked());
        }
        // Extract the updated property.
        // Overwrites date set from published.
        date = readTextNode(itemNode, "updated", deallocate);
        if (date) {
            Nan::Set(item, Nan::New<String>("date").ToLocalChecked(),
                Nan::New<String>(date).ToLocalChecked());
        }
        // Extract the item author.
        parseAtomAuthor(itemNode, item, deallocate);
        if (extractContent) {
            // Extract the item summary.
            char const *summary = readTextNode(itemNode, "summary", deallocate);
            if (summary) {
                Nan::Set(item, Nan::New<String>("summary").ToLocalChecked(),
                    Nan::New<String>(summary).ToLocalChecked());
            }
            // Extract the item content.
            char const *content = readTextNode(itemNode, "content", deallocate);
            if (content) {
                Nan::Set(item, Nan::New<String>("content").ToLocalChecked(),
                    Nan::New<String>(content).ToLocalChecked());
            }
        }
        // Extracts extensions when configured to.
        if (extractExtensions) {
            doExtractExtensions(itemNode, item, deallocate);
        }
        items->Set(Nan::New<Number>(i), item);
        itemNode = itemNode->next_sibling("entry");
        i++;
    }
    Nan::Set(feed, Nan::New<String>("items").ToLocalChecked(), items);
    // Free created buffers.
    deallocateStrings(deallocate);
}

// Creates an array of strings which represent the categories of an item node.

Local<Array> readCategoriesFromItemNode(const xml_node<char> *itemNode, std::vector<char*> &deallocate) {
    Local<Array> categories = Nan::New<Array>();
    xml_node<char> *categoryNode = itemNode->first_node("category");
    int categoryIndex = 0;
    while (categoryNode) {
        char const *category = readTextNode(categoryNode, deallocate);
        if (category) {
            Local<Object> categoryObject = Nan::New<Object>();
            categoryObject->Set(Nan::New<String>("name").ToLocalChecked(),
                Nan::New<String>(category).ToLocalChecked());
            categories->Set(Nan::New<Number>(categoryIndex), categoryObject);
        }
        categoryNode = categoryNode->next_sibling("category");
        categoryIndex++;
    }
    return categories;
}

// Parses the RSS feed.

void parseRssFeed(xml_node<char> *rssNode, const Local<Object> &feed, bool extractContent, bool extractExtensions) {
    // Vector string pointers not
    // deallocated by RapidXML.
    std::vector<char*> deallocate;
    Nan::Set(feed, Nan::New<String>("type").ToLocalChecked(),
        Nan::New<String>("rss").ToLocalChecked());
    xml_node<char> *channelNode = rssNode->first_node("channel");
    if (!channelNode) {
        Nan::ThrowTypeError("Invalid RSS channel.");
        return;
    }
    // Extracts the title property.
    char const *title = readTextNode(channelNode, "title", deallocate);
    if (title) {
        Nan::Set(feed, Nan::New<String>("title").ToLocalChecked(),
            Nan::New<String>(title).ToLocalChecked());
    }
    // Extracts the description property.
    char const *description = readTextNode(channelNode, "description", deallocate);
    if (description) {
        Nan::Set(feed, Nan::New<String>("description").ToLocalChecked(),
            Nan::New<String>(description).ToLocalChecked());
    }
    // Extracts the link property.
    char const *link = readTextNode(channelNode, "link", deallocate);
    if (link) {
        Nan::Set(feed, Nan::New<String>("link").ToLocalChecked(),
            Nan::New<String>(link).ToLocalChecked());
    }
    // Extracts the author property.
    char const *author = readTextNode(channelNode, "author", deallocate);
    if (author) {
        Nan::Set(feed, Nan::New<String>("author").ToLocalChecked(),
            Nan::New<String>(author).ToLocalChecked());
    }
    // Extracts extensions when configured to.
    if (extractExtensions) {
        doExtractExtensions(channelNode, feed, deallocate);
    }
    // Extract all channel items.
    Local<Array> items = Nan::New<Array>();
    xml_node<char> *itemNode = channelNode->first_node("item");
    int i = 0;
    while (itemNode) {
        Local<Object> item = Nan::New<Object>();
        // Extracts the categories.
        Local<Array> categories = readCategoriesFromItemNode(itemNode, deallocate);
        if (categories->Length() > 0) {
            Nan::Set(item, Nan::New<String>("categories").ToLocalChecked(), categories);
        }
        // Extracts the guid property.
        char const *guid = readTextNode(itemNode, "guid", deallocate);
        if (guid) {
            Nan::Set(item, Nan::New<String>("id").ToLocalChecked(),
                Nan::New<String>(guid).ToLocalChecked());
        }
        // Extracts the link property.
        char const *link = readTextNode(itemNode, "link", deallocate);
        if (link) {
            Nan::Set(item, Nan::New<String>("link").ToLocalChecked(),
                Nan::New<String>(link).ToLocalChecked());
        }
        // Extracts the pubDate property.
        char const *date = readTextNode(itemNode, "pubDate", deallocate);
        if (date) {
            Nan::Set(item, Nan::New<String>("date").ToLocalChecked(),
                Nan::New<String>(date).ToLocalChecked());
        }
        // Sometimes given in Dublin Core extension.
        date = readTextNode(itemNode, "dc:date", deallocate);
        if (date) {
            Nan::Set(item, Nan::New<String>("date").ToLocalChecked(),
                Nan::New<String>(date).ToLocalChecked());
        }
        // Extract the item title.
        char const *title = readTextNode(itemNode, "title", deallocate);
        if (title) {
            Nan::Set(item, Nan::New<String>("title").ToLocalChecked(),
                Nan::New<String>(title).ToLocalChecked());
        }
        // Extract the item author.
        char const *author = readTextNode(itemNode, "author", deallocate);
        if (author) {
            Nan::Set(item, Nan::New<String>("author").ToLocalChecked(),
                Nan::New<String>(author).ToLocalChecked());
        }
        // Extract the enclosure if it is set.
        doExtractEnclosure(itemNode, item);
        if (extractContent) {
            // Extract the item description.
            char const *description = readTextNode(itemNode, "description", deallocate);
            if (description) {
                Nan::Set(item, Nan::New<String>("description").ToLocalChecked(),
                    Nan::New<String>(description).ToLocalChecked());
            }
            // <content:encoded> is a popular RSS extension.
            // More info: https://developer.mozilla.org/en-US/docs/Web/RSS/Article/Why_RSS_Content_Module_is_Popular_-_Including_HTML_Contents
            char const *content = readTextNode(itemNode, "content:encoded", deallocate);
            if (content) {
                Nan::Set(item, Nan::New<String>("content").ToLocalChecked(),
                    Nan::New<String>(content).ToLocalChecked());
            }
        }
        // Extracts extensions when configured to.
        if (extractExtensions) {
            doExtractExtensions(itemNode, item, deallocate);
        }
        items->Set(Nan::New<Number>(i), item);
        itemNode = itemNode->next_sibling("item");
        i++;
    }
    Nan::Set(feed, Nan::New<String>("items").ToLocalChecked(), items);
    // Free created buffers.
    deallocateStrings(deallocate);
}

NAN_METHOD(ParseFeed) {
    if (info.Length() < 1) {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }
    Nan::Utf8String xml(info[0]);
    xml_document<char> doc;
    try {
        doc.parse<0>(*xml);
    } catch(rapidxml::parse_error &e) {
        std::pair<int, int> loc = findErrorLine(*xml, e.where<char>());
        std::stringstream err;
        err << "Error on line " << loc.first;
        err << ", column " << loc.second;
        err << ": " << e.what();
        std::string strErr = err.str();
        Nan::ThrowTypeError(strErr.c_str());
        return;
    }
    bool extractContent = true;
    if (info.Length() >= 2) {
        extractContent = info[1]->BooleanValue();
    }
    bool extractExtensions = false;
    if (info.Length() >= 3) {
        extractExtensions = info[2]->BooleanValue();
    }
    // Creates new object to store the feed
    // contents.
    Local<Object> feed = Nan::New<Object>();
    // Tries to get either <rss> or <feed> node.
    xml_node<> *rssNode = doc.first_node("rss");
    if (rssNode) {
        parseRssFeed(rssNode, feed, extractContent, extractExtensions);
    } else {
        xml_node<> *feedNode = doc.first_node("feed");
        if (feedNode) {
            parseAtomFeed(feedNode, feed, extractContent, extractExtensions);
        } else {
            Nan::ThrowTypeError("Invalid feed.");
            return;
        }
    }
    info.GetReturnValue().Set(feed);
}

void init(Handle<Object> exports) {
  Nan::Set(exports, Nan::New<String>("parse").ToLocalChecked(),
      Nan::New<FunctionTemplate>(ParseFeed)->GetFunction());
}

NODE_MODULE(parser, init)
