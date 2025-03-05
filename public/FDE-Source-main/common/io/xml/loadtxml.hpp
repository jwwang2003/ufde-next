#ifndef LOADTXML_HPP
#define LOADTXML_HPP

#include "loadxml.hpp"
#include "tnetlist.hpp"

namespace COS {
namespace XML {

class TNetlistHandler : public NetlistHandler {
protected:
  Port *load_port(xml_node *node, Module *mod);

private:
  void load_timing(xml_node *node, TPort *port);
};

} // namespace XML
} // namespace COS

#endif
