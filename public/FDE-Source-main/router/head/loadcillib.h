#ifndef _RTLOADCIL_H_
#define _RTLOADCIL_H_

#include "cillib.h"
#include "xmlutils.h"

namespace FDU {
namespace rt_cil_lib {
using namespace FDU::XML;

class CilLibHandler {
  cilLibrary *_cilLib;

public:
  CilLibHandler(cilLibrary *cil = 0) : _cilLib(cil) {}
  cilLibrary *loadCil(xml_node *node);

private:
  void loadEleLib(xml_node *node);

  // element Lib
  void loadElem(xml_node *node, elemLib *lib);
  void loadSrams(xml_node *node, Element *ele);
  void loadPaths(xml_node *node, Element *ele);
  void loadSram(xml_node *node, contSramsElem *srams);
  void loadPath(xml_node *node, contPathsElem *paths);
};
} // namespace rt_cil_lib
} // namespace FDU

#endif