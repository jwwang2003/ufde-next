#ifndef XMLUTILS_H
#define XMLUTILS_H

#include <boost/format.hpp>
#include <rapidxml/rapidxml.hpp>
#include <string>

#define foreach_child(child, node, name)                                       \
  for (xml_node *child = node->first_node(name); child;                        \
       child = child->next_sibling(name))

#define foreach_attribute(attr, node)                                          \
  for (xml_attribute *attr = node->first_attribute(); attr;                    \
       attr = attr->next_attribute())

namespace FDU {

using std::string;

template <typename FT, typename VT>
inline string to_string(FT fmt, const VT &val) {
  return (boost::format(fmt) % val).str();
}
template <typename T> inline string to_string(const T &val) {
  return to_string("%1%", val);
}
inline string to_string(double val) { return to_string("%#g", val); }

namespace XML {

using xml_document = rapidxml::xml_document<>;
using xml_node = rapidxml::xml_node<>;
using xml_attribute = rapidxml::xml_attribute<>;

enum CopyType { COPY_NONE = 0, COPY_NAME, COPY_VALUE, COPY_ALL };

inline void set_attribute(xml_node *node, const char *name, const char *value,
                          int copy_string = COPY_NONE) {
  if (copy_string & COPY_NAME)
    name = node->document()->allocate_string(name);
  if (copy_string & COPY_VALUE)
    value = node->document()->allocate_string(value);
  xml_attribute *attr = node->document()->allocate_attribute(name, value);
  node->append_attribute(attr);
}

inline void set_attribute(xml_node *node, const char *name, const string &value,
                          int copy_string = COPY_NONE) {
  set_attribute(node, name, value.c_str(), copy_string);
}
inline void set_attribute(xml_node *node, const string &name, const char *value,
                          int copy_string = COPY_NONE) {
  set_attribute(node, name.c_str(), value, copy_string);
}
inline void set_attribute(xml_node *node, const string &name,
                          const string &value, int copy_string = COPY_NONE) {
  set_attribute(node, name.c_str(), value.c_str(), copy_string);
}

template <typename NT, typename VT>
inline void set_attribute(xml_node *node, NT name, const VT &value,
                          int copy_string = COPY_NONE) {
  set_attribute(node, name, to_string(value), copy_string | COPY_VALUE);
}

inline string get_attribute(xml_node *node, const char *name) {
  xml_attribute *attr = node->first_attribute(name);
  return attr ? attr->value() : "";
}
inline string get_attribute(xml_node *node, const string &name) {
  return get_attribute(node, name.c_str());
}

class DomBuilder {
public:
  xml_document &document() { return doc_; }
  const xml_document &document() const { return doc_; }

  xml_node *create_element(xml_node *node, const char *name,
                           int copy_string = COPY_NONE);
  xml_node *create_element(xml_node *node, const string &name,
                           int copy_string = COPY_NONE) {
    return create_element(node, name.c_str(), copy_string);
  }

  xml_node *create_data(xml_node *node, const char *text,
                        int copy_string = COPY_NONE);
  xml_node *create_data(xml_node *node, const string &text,
                        int copy_string = COPY_NONE) {
    return create_data(node, text.c_str(), copy_string);
  }

  xml_node *create_declaration(const char *version,
                               int copy_string = COPY_NONE);
  xml_node *create_declaration(const string &version,
                               int copy_string = COPY_NONE) {
    return create_declaration(version.c_str(), copy_string);
  }

  // for report
  xml_node *create_pi(xml_node *node, const char *name, const char *value,
                      int copy_string = COPY_NONE);
  xml_node *create_pi(xml_node *node, const char *name, const string &value,
                      int copy_string = COPY_NONE) {
    return create_pi(node, name, value.c_str(), copy_string);
  }
  xml_node *create_pi(xml_node *node, const string &name, const char *value,
                      int copy_string = COPY_NONE) {
    return create_pi(node, name.c_str(), value, copy_string);
  }
  xml_node *create_pi(xml_node *node, const string &name, const string &value,
                      int copy_string = COPY_NONE) {
    return create_pi(node, name.c_str(), value.c_str(), copy_string);
  }

private:
  xml_document doc_;
};

inline xml_node *DomBuilder::create_element(xml_node *node, const char *name,
                                            int copy_string) {
  if (copy_string)
    name = document().allocate_string(name);
  xml_node *elem = document().allocate_node(rapidxml::node_element, name);
  node->append_node(elem);
  return elem;
}

inline xml_node *DomBuilder::create_data(xml_node *node, const char *text,
                                         int copy_string) {
  if (copy_string)
    text = document().allocate_string(text);
  xml_node *data = document().allocate_node(rapidxml::node_data, 0, text);
  node->append_node(data);
  return data;
}

inline xml_node *DomBuilder::create_declaration(const char *version,
                                                int copy_string) {
  if (copy_string)
    version = document().allocate_string(version);
  xml_node *decl = document().allocate_node(rapidxml::node_declaration);
  document().append_node(decl);
  set_attribute(decl, "version", version);
  return decl;
}

inline xml_node *DomBuilder::create_pi(xml_node *node, const char *name,
                                       const char *value, int copy_string) {
  if (copy_string & COPY_NAME)
    name = document().allocate_string(name);
  if (copy_string & COPY_VALUE)
    value = document().allocate_string(value);
  xml_node *pi = document().allocate_node(rapidxml::node_pi, name, value);
  node->append_node(pi);
  return pi;
}

} // namespace XML
} // namespace FDU

#endif
