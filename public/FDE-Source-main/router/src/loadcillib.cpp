#include "cillib.h"
#include "log.h"
#include "zfstream.h"

#include "loadcillib.h"
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <rapidxml/rapidxml_utils.hpp>

namespace FDU {
namespace rt_cil_lib {
using boost::lexical_cast;

cilLibrary *CilLibHandler::loadCil(xml_node *node) {
  ASSERT(_cilLib, "cilLib pointer is nullptr");
  foreach_child(eleLibNode, node, "element_library") loadEleLib(eleLibNode);

  return _cilLib;
}

void CilLibHandler::loadEleLib(xml_node *node) {
  foreach_child(eleNode, node, "element")
      loadElem(eleNode, _cilLib->getElemLib());
}

void CilLibHandler::loadElem(xml_node *node, elemLib *lib) {
  Element *ele = new Element(get_attribute(node, "name"));
  lib->addElement(ele);
  foreach_child(srams, node, "sram_info") loadSrams(srams, ele);
  foreach_child(paths, node, "path_info") loadPaths(paths, ele);
}

void CilLibHandler::loadSrams(xml_node *node, Element *ele) {
  foreach_child(sram, node, "sram") loadSram(sram, &ele->getSrams());
}

void CilLibHandler::loadPaths(xml_node *node, Element *ele) {
  foreach_child(path, node, "path") loadPath(path, &ele->getPaths());
}

void CilLibHandler::loadPath(xml_node *node, contPathsElem *paths) {
  string seg(get_attribute(node, "segregated"));
  bool segBool = false;
  if (seg[0] != 'n')
    segBool = true;
  pathElem *path = new pathElem("", get_attribute(node, "in"),
                                get_attribute(node, "out"), segBool);
  paths->addPath(path);

  foreach_child(cfgs, node, "configuration_info") {
    foreach_child(cfg, cfgs, "sram") loadSram(cfg, &path->getCfgInfo());
  }
}

void CilLibHandler::loadSram(xml_node *node, contSramsElem *srams) {
  string name(get_attribute(node, "name"));
  string dft(get_attribute(node, "default"));
  if (dft == "") {
    int ctt = lexical_cast<int>(get_attribute(node, "content"));
    srams->addSram(new sramElem(false, ctt, name));
  } else {
    int ctt = lexical_cast<int>(dft);
    srams->addSram(new sramElem(true, ctt, name));
  }
}

} // namespace rt_cil_lib
} // namespace FDU

namespace FDU {
namespace rt_cil_lib {
using namespace FDU::XML;

typedef rapidxml::file<> xml_file;

cilLibrary *cilLibrary::loadCilLib(const std::string &file) {
  cilLibrary *lib = new cilLibrary();
  CilLibHandler cilh{lib};
  xml_document doc;
  ifstrm ifs(file.c_str());
  xml_file xfile(ifs);
  doc.parse<0>(xfile.data());
  cilh.loadCil(doc.first_node());
  return lib;
}
} // namespace rt_cil_lib
} // namespace FDU