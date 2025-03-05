#ifndef LOADXML_HPP
#define LOADXML_HPP

#include "io/fileio.hpp"
#include "netlist.hpp"
#include "xmlutils.h"

namespace COS {
namespace XML {

using namespace FDU::XML;

class NetlistHandler {
public:
  virtual Design *load_design(xml_node *node, Design *design);
  virtual Design *load_libs(xml_node *node, Design *design);
  virtual Library *load_library(xml_node *node, Design *design,
                                bool external = true);

protected:
  virtual Module *load_module(xml_node *node, Library *lib);
  virtual Port *load_port(xml_node *node, Module *mod);
  virtual Instance *load_instance(xml_node *node, Module *mod);
  virtual Net *load_net(xml_node *node, Module *mod);
  virtual Bus *load_bus(xml_node *node, Module *mod);
  virtual Pin *load_pin(xml_node *node, Net *net);
  virtual void load_bus_port(xml_node *node, Bus *bus);

private:
  void load_properties(xml_node *node, Object *obj);
};

} // namespace XML
} // namespace COS

#endif
