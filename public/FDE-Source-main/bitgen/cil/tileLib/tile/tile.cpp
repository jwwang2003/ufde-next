#include "cil/tileLib/tile/tile.h"

#include <boost/lexical_cast.hpp>

namespace FDU {
namespace cil_lib {

void Tile::construct() {
  _trnss.setRefTrnsLib(_refTrnsLib);
  _trnss.setRefTileName(getName());
  _clsts.setRefClstLib(_refClstLib);
  _clsts.setRefTileName(getName());
}

void Tile::getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement) {
  getRefSite(cfgElement._siteName)->getContentsOfFunc(bits, cfgElement);
}

void Tile::getPathPips(vecBits &bits, const routeInfo &route) {
  getRefSite(route._siteName)->getPathPips(bits, route);
}

void Tile::setBitPlace(bitTile &bit) {
  try {
    sramInSiteSramTile *refSram = getRefSram(bit);
    bit._localPlace = refSram->getLocalPlace();
    bit._tileOffset = refSram->getTileOffset();
    bit._landLordTile = refSram->getLandlordTile();
    bit._ownerTile = refSram->getOwnerTile();
  } catch (Exception &e) {
    throw CilException(string(e.what()).substr(6) + " at site " +
                       bit._siteName + " in tile " + _name);
  }
}

} // namespace cil_lib
} // namespace FDU