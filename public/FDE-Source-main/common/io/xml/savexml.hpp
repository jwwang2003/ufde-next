#ifndef SAVEXML_HPP
#define SAVEXML_HPP

#include "io/fileio.hpp"
#include "netlist.hpp"
#include "xmlutils.h"

namespace COS {
namespace XML {

using namespace FDU::XML;

class NetlistWriter : public DomBuilder {
public:
  virtual xml_node *write_design(const Design *design);
  virtual xml_node *write_libs(const Design *design);

protected:
  virtual xml_node *write_library(xml_node *des_node, const Library *lib);
  virtual xml_node *write_module(xml_node *lib_node, const Module *mod);
  virtual xml_node *write_port(xml_node *mod_node, const Port *port);
  virtual xml_node *write_instance(xml_node *mod_node, const Instance *inst);
  virtual xml_node *write_net(xml_node *mod_node, const Net *net);
  virtual xml_node *write_bus(xml_node *mod_node, const Bus *bus);
  virtual xml_node *write_pin(xml_node *net_node, const Pin *pin);

  virtual void write_properties(xml_node *node, const Object *obj);
  virtual xml_node *write_property(xml_node *node, PropertyBase *prop,
                                   const Object *obj);
  virtual xml_node *write_config(xml_node *node, Config *cfg,
                                 const Object *obj);

  using DomBuilder::create_element;
  xml_node *create_element(xml_node *node, const char *name, const Object *obj,
                           int copy_string = COPY_NONE);
  xml_node *create_element(xml_node *node, const string &name,
                           const Object *obj, int copy_string = COPY_NONE) {
    return create_element(node, name.c_str(), obj, copy_string);
  }
};

} // namespace XML
} // namespace COS

#endif
