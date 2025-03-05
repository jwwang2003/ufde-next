#include "loadxml.hpp"

namespace COS {
XML::NetlistHandler *CosFactory::make_xml_read_handler() {
  return new XML::NetlistHandler;
}

namespace XML {
using namespace boost;

Design *NetlistHandler::load_libs(xml_node *node, Design *design) {
  foreach_child(lib_node, node, "external") load_library(lib_node, design);
  foreach_child(lib_node, node, "library") load_library(lib_node, design);
  return design;
}

Design *NetlistHandler::load_design(xml_node *node, Design *design) {
  design->rename(get_attribute(node, "name"));

  foreach_child(lib_node, node, "external") load_library(lib_node, design);
  foreach_child(lib_node, node, "library")
      load_library(lib_node, design, false);

  xml_node *top_node = node->first_node("topModule");
  if (!top_node)
    top_node = node->first_node("topCell"); // old netlist
  ASSERT(top_node, "parse failed. design: no top module");

  string alref = get_attribute(top_node, "libraryRef");
  Library *lref = design->find_library(alref);
  ASSERT(lref, "parse failed. topModule: library not found");
  string aname = get_attribute(top_node, "name");
  Module *mod = lref->find_module(aname);
  ASSERT(mod, "parse failed. topModule: cell not found");
  design->set_top_module(mod);

  load_properties(node, design);
  return design;
}

Library *NetlistHandler::load_library(xml_node *node, Design *design,
                                      bool external) {
  Library *lib =
      design->find_or_create_library(get_attribute(node, "name"), external);
  ASSERT(lib, "parse failed. library not created");
  foreach_child(mod_node, node, "module") load_module(mod_node, lib);
  foreach_child(mod_node, node, "cell") // old netlist
      load_module(mod_node, lib);
  load_properties(node, lib);
  return lib;
}

Module *NetlistHandler::load_module(xml_node *node, Library *lib) {
  string aname = get_attribute(node, "name");
  string atype = get_attribute(node, "type");
  if (lib->find_module(aname)) { // module already exist
    if (lib->is_external())
      return 0;
    ASSERT(0, "parse failed. module already exist");
  }

  Module *mod = lib->create_module(aname, atype);
  ASSERT(mod, "parse failed. module not created");
  foreach_child(port_node, node, "port") load_port(port_node, mod);
  foreach_child(port_node, node, "portGrp") // old netlist
      load_port(port_node, mod);

  load_properties(node, mod);

  xml_node *contents_node = node->first_node("contents");
  if (!contents_node)
    return mod;

  foreach_child(inst_node, contents_node, "instance") {
    Instance *inst = load_instance(inst_node, mod);
    if (!inst && atype == "RULE_CELL")
      return mod;
    ASSERT(inst, "parse failed. module not found");
  }
  foreach_child(net_node, contents_node, "net") load_net(net_node, mod);

  foreach_child(bus_node, contents_node, "bus") load_bus(bus_node, mod);

  return mod;
}

Port *NetlistHandler::load_port(xml_node *node, Module *mod) {
  string aname = get_attribute(node, "name");
  DirType dir = lexical_cast<DirType>(get_attribute(node, "direction"));
  PortType type = lexical_cast<PortType>(get_attribute(node, "type"));
  string amsb = get_attribute(node, "msb");
  string alsb = get_attribute(node, "lsb");
  Port *port;
  if (amsb.empty() || alsb.empty()) {
    port = mod->create_port(aname, dir, type);
  } else {
    int msb = lexical_cast<int>(amsb);
    int lsb = lexical_cast<int>(alsb);
    port = mod->create_port(aname, msb, lsb, dir, type);
  }
  ASSERT(port, "parse failed. port not created");
  load_properties(node, port);
  return port;
}

Instance *NetlistHandler::load_instance(xml_node *node, Module *mod) {
  string aname = get_attribute(node, "name");
  string amref = get_attribute(node, "moduleRef");
  if (amref.empty())
    amref = get_attribute(node, "cellRef");
  string alref = get_attribute(node, "libraryRef");
  Library *lib = mod->library();
  if (!alref.empty()) {
    Library *lref = lib->design()->find_library(alref);
    ASSERT(lref, "parse failed. libraryRef: library not found");
    lib = lref;
  }
  Module *dmod = lib->find_module(amref);
  //		ASSERTM(dmod, "parse failed. moduleRef: module not found");
  if (!dmod)
    return 0; // for pack rule
  Instance *inst = mod->create_instance(aname, dmod);
  ASSERT(inst, "parse failed. instance not created");
  load_properties(node, inst);
  return inst;
}

Net *NetlistHandler::load_net(xml_node *node, Module *mod) {
  string aname = get_attribute(node, "name");
  NetType type = lexical_cast<NetType>(get_attribute(node, "type"));
  Net *net = mod->create_net(aname, type);
  ASSERT(net, "parse failed. net not created");
  foreach_child(pin_node, node, "portRef") load_pin(pin_node, net);
  load_properties(node, net);
  return net;
}

Bus *NetlistHandler::load_bus(xml_node *node, Module *mod) {
  string aname = get_attribute(node, "name");
  NetType type = lexical_cast<NetType>(get_attribute(node, "type"));
  int msb = lexical_cast<int>(get_attribute(node, "msb"));
  int lsb = lexical_cast<int>(get_attribute(node, "lsb"));
  Bus *bus = mod->create_bus(aname, msb, lsb, type);
  ASSERT(bus, "parse failed. net not created");
  foreach_child(port_node, node, "portRef") load_bus_port(port_node, bus);
  load_properties(node, bus);
  return bus;
}

Pin *NetlistHandler::load_pin(xml_node *node, Net *net) {
  Module *mod = net->module();
  string aname = get_attribute(node, "name");
  string ainst = get_attribute(node, "instanceRef");
  Pin *pin;
  if (ainst.empty()) { // mpin
    pin = mod->find_pin(aname);
  } else {
    Instance *inst = mod->find_instance(ainst);
    ASSERT(inst, "parse failed. portRef: instance not found");
    pin = inst->find_pin(aname);
  }
  ASSERT(pin, "parse failed. portRef: pin not found");
  pin->connect(net);
  load_properties(node, pin);
  return pin;
}

void NetlistHandler::load_bus_port(xml_node *node, Bus *bus) {
  Module *mod = bus->module();
  string aname = get_attribute(node, "name");
  string ainst = get_attribute(node, "instanceRef");
  ConnectDir dir = lexical_cast<ConnectDir>(get_attribute(node, "connect"));
  Port *port = mod->find_port(aname);
  if (ainst.empty())
    bus->connect(port, dir);
  else {
    Instance *inst = mod->find_instance(ainst);
    ASSERT(inst, "parse failed. portRef: instance not found");
    bus->connect(inst, port, dir);
  }
  // to do: property
}

void NetlistHandler::load_properties(xml_node *node, Object *obj) {
  ObjectClass cls = obj->class_id();
  foreach_child(prop_node, node, "property") {
    string name = get_attribute(prop_node, "name");
    string value = get_attribute(prop_node, "value");
    string type = get_attribute(prop_node, "type");
    PropertyBase *prop = find_property(cls, name);
    if (!prop) {
      if (type == "" || type == "string")
        prop = &create_property<string>(cls, name);
      else if (type == "int")
        prop = &create_property<int>(cls, name);
      else if (type == "double")
        prop = &create_property<double>(cls, name);
      else if (type == "point")
        prop = &create_property<Point>(cls, name);
      else
        ASSERT(0, "parse failed. invalid property type");
    }
    prop->set_string_value(obj, value);
  }
  if (cls != INSTANCE)
    return;
  foreach_child(cfg_node, node, "config") {
    string name = get_attribute(cfg_node, "name");
    string value = get_attribute(cfg_node, "value");
    Config &cfg =
        create_config(static_cast<Instance *>(obj)->module_type(), name);
    cfg.set_string_value(obj, value);
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//  Constraint

void load_constraints(xml_node *node, Design *design) {
  foreach_child(port_node, node, "port") {
    Property<string> &pad = create_temp_property<string>(INSTANCE, "pad");
    Property<string> &clk_type = create_property<string>(INSTANCE, "clk_type");
    string name = get_attribute(port_node, "name");
    Instance *inst = design->top_module()->find_instance(name);
    ASSERT(inst, format("parse failed. constraint: %s not found") % name);
    string spos = get_attribute(port_node, "position");
    if (!spos.empty())
      inst->set_property(pad, spos);
    string stype = get_attribute(port_node, "clk_type");
    if (!stype.empty())
      inst->set_property(clk_type, stype);
  }
}

} // namespace XML
} // namespace COS

#include <rapidxml/rapidxml_utils.hpp> // file

namespace COS {
namespace IO {
using namespace COS::XML;

using xml_file = rapidxml::file<>;

class XmlLoader : public Loader { // XML design file
public:
  XmlLoader() : Loader("xml") {}
  void load(std::istream &istrm);
};

void XmlLoader::load(std::istream &istrm) {
  ASSERT(design(), "parse failed. no current design");
  xml_file file(istrm);
  xml_document doc;
  doc.parse<0>(file.data());
  xml_node *node = doc.first_node();
  std::unique_ptr<NetlistHandler> nlh(
      CosFactory::instance().make_xml_read_handler());
  if (node->name() == string("design"))
    nlh->load_design(node, design());
  else if (node->name() == string("library"))
    nlh->load_library(node, design());
  else
    nlh->load_libs(node, design());
}

class ConstraintLoader : public Loader { // Constraint file
public:
  ConstraintLoader() : Loader("constraint") {}
  void load(std::istream &istrm);
};

void ConstraintLoader::load(std::istream &istrm) {
  xml_file file(istrm);
  xml_document doc;
  doc.parse<0>(file.data());
  load_constraints(doc.first_node(), design());
}

static XmlLoader xml_loader;
static ConstraintLoader cst_loader;

} // namespace IO
} // namespace COS
