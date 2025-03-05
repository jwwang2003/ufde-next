#ifndef SAVETXML_HPP
#define SAVETXML_HPP

#include "savexml.hpp"
#include "tnetlist.hpp"

namespace COS {
namespace XML {

class TNetlistWriter : public NetlistWriter {
protected:
  xml_node *write_port(xml_node *mod_node, const Port *port);

private:
  xml_node *write_timing(xml_node *node, const TimingInfo &tinfo);
};

} // namespace XML
} // namespace COS

#endif
