#include "savetxml.hpp"

namespace COS {
XML::NetlistWriter *TimingFactory::make_xml_write_handler() {
  return new XML::TNetlistWriter;
}

namespace XML {
using boost::format;
using boost::str;

xml_node *TNetlistWriter::write_port(xml_node *mod_node, const Port *port) {
  xml_node *port_node = NetlistWriter::write_port(mod_node, port);
  const TPort *tport = dynamic_cast<const TPort *>(port);
  if (tport->capacitance() >= 0.)
    set_attribute(port_node, "capacitance", tport->capacitance());
  for (const TimingInfo &t : tport->timings())
    write_timing(port_node, t);
  return port_node;
}

xml_node *TNetlistWriter::write_timing(xml_node *node,
                                       const TimingInfo &tinfo) {
  xml_node *timing_node = create_element(node, "timing");
  if (!tinfo.timing_type.empty())
    set_attribute(timing_node, "timing_type", tinfo.timing_type);
  if (!tinfo.related_pin.empty())
    set_attribute(timing_node, "related_pin", tinfo.related_pin);
  if (!tinfo.timing_sense.empty())
    set_attribute(timing_node, "timing_sense", tinfo.timing_sense);
  if (tinfo.intrinsic_rise != 0.0)
    set_attribute(timing_node, "intrinsic_rise", tinfo.intrinsic_rise);
  if (tinfo.intrinsic_fall != 0.0)
    set_attribute(timing_node, "intrinsic_fall", tinfo.intrinsic_fall);
  return timing_node;
}

} // namespace XML
} // namespace COS
