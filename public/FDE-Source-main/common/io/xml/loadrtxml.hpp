#ifndef LOADRTXML_HPP
#define LOADRTXML_HPP

#include "loadtxml.hpp"
#include "rtnetlist.hpp"

namespace COS {
namespace XML {

class RTNetlistHandler : public TNetlistHandler {
protected:
  Net *load_net(xml_node *node, Module *mod);

private:
  void load_pip(xml_node *node, COSRTNet *net);
};

} // namespace XML
} // namespace COS

#endif
