#include "loadrtxml.hpp"

namespace COS {
XML::NetlistHandler *COSRTFactory::make_xml_read_handler() {
  return new XML::RTNetlistHandler;
}

namespace XML {
using boost::lexical_cast;

Net *RTNetlistHandler::load_net(xml_node *node, Module *mod) {
  COSRTNet *net = dynamic_cast<COSRTNet *>(NetlistHandler::load_net(node, mod));
  ASSERT(net, "parse failed. net: cast error");
  foreach_child(pip_node, node, "pip") load_pip(pip_node, net);
  return net;
}

void RTNetlistHandler::load_pip(xml_node *node, COSRTNet *net) {
  string from = get_attribute(node, "from");
  string to = get_attribute(node, "to");
  Point pos = lexical_cast<Point>(get_attribute(node, "position"));
  string adir = get_attribute(node, "dir");
  PIP::pip_dir dir =
      adir.empty() ? PIP::unidir : lexical_cast<PIP::pip_dir>(adir);
  net->create_pip(from, to, pos, dir);
}

} // namespace XML
} // namespace COS
