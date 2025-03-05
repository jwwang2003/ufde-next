#include "cil/tileLib/tileLib.h"

// #include <boost/foreach.hpp>

namespace FDU {
namespace cil_lib {

void tileLib::listDfotCfgs(vecCfgs &cfgs) {
  vecCfgs tCfgs;
  for (Tile *tile : tiles()) {
    tile->listDfotCfgs(tCfgs);
    for (cfgElem cfg : tCfgs) {
      cfg._tileName = tile->getName();
      cfgs.push_back(cfg);
    }
    tCfgs.clear();
  }
}

} // namespace cil_lib
} // namespace FDU