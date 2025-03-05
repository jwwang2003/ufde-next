#include "savertxml.hpp"

namespace COS {
namespace XML {

class ONetlistWriter : public RTNetlistWriter {
public:
  xml_node *write_design(const Design *design);

protected:
  xml_node *write_module(xml_node *lib_node, const Module *mod);
  xml_node *write_port(xml_node *mod_node, const Port *port);
  xml_node *write_instance(xml_node *mod_node, const Instance *inst);

  xml_node *write_property(xml_node *node, PropertyBase *prop,
                           const Object *obj);
  xml_node *write_config(xml_node *node, Config *cfg, const Object *obj);

private:
  xml_node *write_timing(xml_node *node, const TimingInfo *tinfo);
};

xml_node *ONetlistWriter::write_property(xml_node *node, PropertyBase *prop,
                                         const Object *obj) {
  xml_node *prop_node = create_element(node, "property");
  set_attribute(prop_node, "name", prop->name());
  set_attribute(prop_node, "value", prop->string_value(obj), COPY_VALUE);
  return prop_node;
}

xml_node *ONetlistWriter::write_config(xml_node *node, Config *cfg,
                                       const Object *obj) {
  return write_property(node, cfg, obj);
}

xml_node *ONetlistWriter::write_design(const Design *design) {
  xml_node *des_node = create_element(&document(), "design", design);
  for (const Library *lib : IO::Writer::used_lib(design))
    write_library(des_node, lib);

  const Module *top = design->top_module();
  xml_node *top_node = create_element(des_node, "topCell"); // old tag
  set_attribute(top_node, "name", top->name());
  set_attribute(top_node, "libraryRef", top->library()->name());

  return des_node;
}

xml_node *ONetlistWriter::write_module(xml_node *lib_node, const Module *mod) {
  xml_node *mod_node = create_element(lib_node, "cell", mod); // old tag
  if (!mod->type().empty())
    set_attribute(mod_node, "type", mod->type());
  for (const Port *port : mod->ports())
    write_port(mod_node, port);
  if (mod->is_composite() && lib_node->name() == string("library")) {
    xml_node *cont_node = create_element(mod_node, "contents");
    for (const Instance *inst : mod->instances())
      write_instance(cont_node, inst);
    for (const Net *net : mod->nets())
      write_net(cont_node, net);
  }
  return mod_node;
}

xml_node *ONetlistWriter::write_port(xml_node *mod_node, const Port *port) {
  xml_node *port_node = 0;
  if (port->is_vector()) {
    port_node = create_element(mod_node, "portGrp", port);
    set_attribute(port_node, "msb", port->msb());
    set_attribute(port_node, "lsb", port->lsb());
  } else
    port_node = create_element(mod_node, "port", port);
  set_attribute(port_node, "direction", port->dir());
  if (port->type() != NORMAL)
    set_attribute(port_node, "type", port->type());
  const TPort *tport = dynamic_cast<const TPort *>(port);
  if (tport->capacitance() >= 0.)
    set_attribute(port_node, "capacitance", tport->capacitance());
  for (const TimingInfo &t : tport->timings())
    write_timing(port_node, &t);
  return port_node;
}

xml_node *ONetlistWriter::write_instance(xml_node *mod_node,
                                         const Instance *inst) {
  xml_node *inst_node = create_element(mod_node, "instance", inst);
  const Module *module = inst->down_module();
  const Library *lib = module->library();
  set_attribute(inst_node, "cellRef", module->name());
  if (lib != inst->module()->library())
    set_attribute(inst_node, "libraryRef", lib->name());
  return inst_node;
}

xml_node *ONetlistWriter::write_timing(xml_node *node,
                                       const TimingInfo *tinfo) {
  xml_node *timing_node = create_element(node, "timing");
  if (!tinfo->related_pin.empty())
    set_attribute(create_element(timing_node, "related_pin"), "value",
                  tinfo->related_pin);
  if (!tinfo->timing_sense.empty())
    set_attribute(create_element(timing_node, "timing_sense"), "value",
                  tinfo->timing_sense);
  if (!tinfo->timing_type.empty())
    set_attribute(create_element(timing_node, "timing_type"), "value",
                  tinfo->timing_type);
  if (tinfo->intrinsic_rise != 0.0)
    set_attribute(create_element(timing_node, "intrinsic_rise"), "value",
                  tinfo->intrinsic_rise);
  if (tinfo->intrinsic_fall != 0.0)
    set_attribute(create_element(timing_node, "intrinsic_fall"), "value",
                  tinfo->intrinsic_fall);
  return timing_node;
}

} // namespace XML
} // namespace COS

#include <rapidxml/rapidxml_print.hpp>

namespace COS {
namespace IO {

class OXmlWriter : public Writer {
public:
  OXmlWriter() : Writer("oxml") {}
  void write(std::ostream &ostrm) const;
};

void OXmlWriter::write(std::ostream &ostrm) const {
  mark_used();
  XML::ONetlistWriter nlw;
  nlw.write_design(design());
  ostrm << nlw.document();
}

static OXmlWriter xml_writer;
} // namespace IO
} // namespace COS