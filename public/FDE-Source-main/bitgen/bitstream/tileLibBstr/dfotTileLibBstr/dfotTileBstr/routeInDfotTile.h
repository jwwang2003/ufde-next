#ifndef _ROUTEINDFOTTILE_H_
#define _ROUTEINDFOTTILE_H_

#include "cil/tileLib/tile/tile.h"

#include <string>
#include <vector>

namespace BitGen {
namespace bitstream {

class contRoutesDfotTileBstr {
public:
  using Routes = std::vector<routeInfo>;

private:
  Routes _routes;
  FDU::cil_lib::Tile *_refTile;

public:
  void setRefTile(FDU::cil_lib::Tile *ref) { _refTile = ref; }

  routeInfo &addRoute(const routeInfo &route) {
    _routes.push_back(route);
    return _routes.back();
  }
  Routes &getRoutes() { return _routes; }

  void listDfotPips(vecBits &bits);
  void listCktPips(vecBits &bits);
  void analyzePipPlace(vecBits &bits);
  void checkSiteNameOfPip(vecBits &bits);
};

//////////////////////////////////////////////////////////////////////////
// inline functions

inline void contRoutesDfotTileBstr::listDfotPips(vecBits &bits) {
  _refTile->listDfotPips(bits);
#ifdef _DEBUG
  checkSiteNameOfPip(bits);
#endif
}

inline void contRoutesDfotTileBstr::listCktPips(vecBits &bits) {
  for (const routeInfo &route : _routes)
    _refTile->getPathPips(bits, route);
}

inline void contRoutesDfotTileBstr::analyzePipPlace(vecBits &bits) {
  for (bitTile &bit : bits)
    _refTile->setBitPlace(bit);
}

} // namespace bitstream
} // namespace BitGen

#endif