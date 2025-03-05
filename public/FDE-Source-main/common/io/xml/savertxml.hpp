#ifndef SAVERTXML_HPP
#define SAVERTXML_HPP

#include "rtnetlist.hpp"
#include "savetxml.hpp"

namespace COS {
namespace XML {

class RTNetlistWriter : public TNetlistWriter {
protected:
  xml_node *write_net(xml_node *mod_node, const Net *net);

private:
  xml_node *write_pip(xml_node *node, const PIP *pip);
};

} // namespace XML
} // namespace COS

#endif
