#include "loadtxml.hpp"

namespace COS {
XML::NetlistHandler *TimingFactory::make_xml_read_handler() {
  return new XML::TNetlistHandler;
}

namespace XML {
using boost::lexical_cast;

Port *TNetlistHandler::load_port(xml_node *node, Module *mod) {
  TPort *port = dynamic_cast<TPort *>(NetlistHandler::load_port(node, mod));
  ASSERT(port, "parse failed. port: cast error");
  string avalue = get_attribute(node, "capacitance");
  if (!avalue.empty())
    port->set_capacitance(lexical_cast<double>(avalue));
  foreach_child(timing_node, node, "timing") load_timing(timing_node, port);
  return port;
}

void TNetlistHandler::load_timing(xml_node *node, TPort *port) {
  string type = get_attribute(node, "timing_type");
  string pin = get_attribute(node, "related_pin");
  string sense = get_attribute(node, "timing_sense");
  string srise = get_attribute(node, "intrinsic_rise");
  string sfall = get_attribute(node, "intrinsic_fall");
  double rise = srise.empty() ? 0.0 : lexical_cast<double>(srise);
  double fall = sfall.empty() ? 0.0 : lexical_cast<double>(sfall);

  foreach_child(value_node, node, 0) { // old netlist
    string name = value_node->name();
    string value = get_attribute(value_node, "value");
    if (name == "timing_type")
      type = value;
    else if (name == "related_pin")
      pin = value;
    else if (name == "timing_sense")
      sense = value;
    else if (name == "intrinsic_rise")
      rise = lexical_cast<double>(value);
    else if (name == "intrinsic_fall")
      fall = lexical_cast<double>(value);
  }

  port->create_timing(type, pin, sense, rise, fall);
}

} // namespace XML
} // namespace COS
