#include "RTCstLoadHandler.h"
#include "utils.h"
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <rapidxml/rapidxml_utils.hpp>

namespace FDU {
namespace RT {
using boost::lexical_cast;
using namespace COS;
using namespace XML;
typedef rapidxml::file<> xml_file;
void RTCstLoadHandler::load() {
  ASSERTD(_pCstNets, "_pCstNets is nullptr.");
  std::ifstream cstFile(_cstFileName.c_str());
  xml_file xfile(cstFile);
  xml_document doc;
  doc.parse<0>(xfile.data());
  loadCstNets(doc.first_node());
}

void RTCstLoadHandler::loadCstNets(xml_node *node) {
  _pCstNets->setDesignName(get_attribute(node, "name"));
  foreach_child(cstNetNode, node, "net") loadCstNet(cstNetNode);
}

void RTCstLoadHandler::loadCstNet(xml_node *node) {
  CstNet *net = _pCstNets->creat_cstNet(get_attribute(node, "name"));
  foreach_child(pipNode, node, "pip") loadPip(pipNode, net);
}

void RTCstLoadHandler::loadPip(xml_node *node, CstNet *net) {
  string from = get_attribute(node, "from");
  string to = get_attribute(node, "to");
  Point pos = lexical_cast<Point>(get_attribute(node, "position"));
  string adir = get_attribute(node, "dir");
  PIP::pip_dir dir =
      adir.empty() ? PIP::unidir : lexical_cast<PIP::pip_dir>(adir);
  net->create_pip(from, to, pos, dir);
}
} // namespace RT
} // namespace FDU