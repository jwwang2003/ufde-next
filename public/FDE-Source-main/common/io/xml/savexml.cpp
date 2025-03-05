#include "savexml.hpp"

namespace COS {
XML::NetlistWriter *CosFactory::make_xml_write_handler() {
  return new XML::NetlistWriter;
}

namespace XML {
using namespace COS;

xml_node *NetlistWriter::create_element(xml_node *node, const char *name,
                                        const Object *obj, int copy_string) {
  xml_node *elem = create_element(node, name, copy_string);
  set_attribute(elem, "name", obj->name());
  write_properties(elem, obj);
  return elem;
}

void NetlistWriter::write_properties(xml_node *node, const Object *obj) {
  for (PropertyBase *pp : obj->properties())
    if (pp->store_save() && obj->property_exist(*pp))
      write_property(node, pp, obj);
  if (const Instance *inst = dynamic_cast<const Instance *>(obj))
    for (Config *cfg : inst->configs())
      write_config(node, cfg, obj);
}

xml_node *NetlistWriter::write_property(xml_node *node, PropertyBase *prop,
                                        const Object *obj) {
  string type = prop->value_type();
  ASSERT(!type.empty(), "writexml: invalid property type.");
  xml_node *prop_node = create_element(node, "property");
  set_attribute(prop_node, "name", prop->name());
  if (type != "string")
    set_attribute(prop_node, "type", type, COPY_VALUE);
  set_attribute(prop_node, "value", prop->string_value(obj), COPY_VALUE);
  return prop_node;
}

xml_node *NetlistWriter::write_config(xml_node *node, Config *cfg,
                                      const Object *obj) {
  xml_node *prop_node = create_element(node, "config");
  set_attribute(prop_node, "name", cfg->name());
  set_attribute(prop_node, "value", cfg->string_value(obj), COPY_VALUE);
  return prop_node;
}

xml_node *NetlistWriter::write_design(const Design *design) {
  xml_node *des_node = create_element(&document(), "design", design);
  for (const Library *lib : IO::Writer::used_lib(design))
    write_library(des_node, lib);

  const Module *top = design->top_module();
  xml_node *top_node = create_element(des_node, "topModule");
  set_attribute(top_node, "name", top->name());
  set_attribute(top_node, "libraryRef", top->library()->name());

  return des_node;
}

xml_node *NetlistWriter::write_libs(const Design *design) {
  xml_node *libs_node = create_element(&document(), "libraries");
  for (const Library *lib : design->libs()) {
    xml_node *lib_node = create_element(libs_node, "library", lib);
    for (const Module *mod : lib->modules())
      write_module(lib_node, mod);
  }
  return libs_node;
}

xml_node *NetlistWriter::write_library(xml_node *des_node, const Library *lib) {
  const char *tag = lib->is_external() ? "external" : "library";
  xml_node *lib_node = create_element(des_node, tag, lib);
  for (const Module *mod : IO::Writer::used_module(lib))
    write_module(lib_node, mod);
  return lib_node;
}

xml_node *NetlistWriter::write_module(xml_node *lib_node, const Module *mod) {
  xml_node *mod_node = create_element(lib_node, "module", mod);
  if (!mod->type().empty())
    set_attribute(mod_node, "type", mod->type());
  for (const Port *port : mod->ports())
    write_port(mod_node, port);
  if (mod->is_composite() && lib_node->name() != string("external")) {
    xml_node *cont_node = create_element(mod_node, "contents");
    for (const Instance *inst : mod->instances())
      write_instance(cont_node, inst);
    for (const Net *net : mod->nets())
      write_net(cont_node, net);
  }
  return mod_node;
}

xml_node *NetlistWriter::write_port(xml_node *mod_node, const Port *port) {
  xml_node *port_node = create_element(mod_node, "port", port);
  if (port->is_vector()) {
    set_attribute(port_node, "msb", port->msb());
    set_attribute(port_node, "lsb", port->lsb());
  }
  set_attribute(port_node, "direction", port->dir());
  if (port->type() != NORMAL)
    set_attribute(port_node, "type", port->type());
  return port_node;
}

xml_node *NetlistWriter::write_instance(xml_node *mod_node,
                                        const Instance *inst) {
  xml_node *inst_node = create_element(mod_node, "instance", inst);
  const Module *module = inst->down_module();
  const Library *lib = module->library();
  set_attribute(inst_node, "moduleRef", module->name());
  if (lib != inst->module()->library())
    set_attribute(inst_node, "libraryRef", lib->name());
  return inst_node;
}

xml_node *NetlistWriter::write_net(xml_node *mod_node, const Net *net) {
  xml_node *net_node = create_element(mod_node, "net", net);
  if (net->type() != NORMAL)
    set_attribute(net_node, "type", net->type());
  for (const Pin *pin : net->pins())
    write_pin(net_node, pin);
  return net_node;
}

xml_node *NetlistWriter::write_pin(xml_node *net_node, const Pin *pin) {
  xml_node *pin_node = create_element(net_node, "portRef", pin);
  if (!pin->is_mpin())
    set_attribute(pin_node, "instanceRef", pin->instance()->name());
  return pin_node;
}

xml_node *NetlistWriter::write_bus(xml_node *mod_node, const Bus *bus) {
  xml_node *bus_node = create_element(mod_node, "bus", bus);
  set_attribute(bus_node, "msb", bus->msb());
  set_attribute(bus_node, "lsb", bus->lsb());
  const Net *net0 = *bus->nets().begin();
  if (net0->type() != NORMAL)
    set_attribute(bus_node, "type", net0->type());
  for (const Pin *pin : net0->pins()) {
    xml_node *pin_node = write_pin(bus_node, pin);
    const char *conn_dir = pin->index_within_port() ? "right" : "left";
    set_attribute(pin_node, "connect", conn_dir);
  }
  return bus_node;
}

} // namespace XML
} // namespace COS

#include <rapidxml/rapidxml_print.hpp>

namespace COS {
namespace IO {
using namespace COS::XML;

class XmlWriter : public Writer {
public:
  XmlWriter() : Writer("xml") {}
  void write(std::ostream &ostrm) const;
};

void XmlWriter::write(std::ostream &ostrm) const {
  mark_used();
  std::unique_ptr<NetlistWriter> nlw(
      CosFactory::instance().make_xml_write_handler());
  nlw->write_design(design());
  ostrm << nlw->document();
}

class LibsWriter : public Writer {
public:
  LibsWriter() : Writer("libs") {}
  void write(std::ostream &ostrm) const;
};

void LibsWriter::write(std::ostream &ostrm) const {
  std::unique_ptr<NetlistWriter> nlw(
      CosFactory::instance().make_xml_write_handler());
  nlw->write_libs(design());
  ostrm << nlw->document();
}

static XmlWriter xml_writer;
static LibsWriter libs_writer;

} // namespace IO
} // namespace COS
