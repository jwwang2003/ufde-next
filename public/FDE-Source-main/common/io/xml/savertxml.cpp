#include "savertxml.hpp"

namespace COS {
XML::NetlistWriter *COSRTFactory::make_xml_write_handler() {
  return new XML::RTNetlistWriter;
}

namespace XML {

xml_node *RTNetlistWriter::write_net(xml_node *mod_node, const Net *net) {
  xml_node *net_node = NetlistWriter::write_net(mod_node, net);
  const COSRTNet *rtnet = dynamic_cast<const COSRTNet *>(net);
  for (const PIP *pip : rtnet->routing_pips())
    write_pip(net_node, pip);
  return net_node;
}

xml_node *RTNetlistWriter::write_pip(xml_node *node, const PIP *pip) {
  xml_node *pip_node = create_element(node, "pip");
  set_attribute(pip_node, "from", pip->from());
  set_attribute(pip_node, "to", pip->to());
  set_attribute(pip_node, "position", pip->position());
  set_attribute(pip_node, "dir", pip->dir());
  return pip_node;
}

} // namespace XML
} // namespace COS
