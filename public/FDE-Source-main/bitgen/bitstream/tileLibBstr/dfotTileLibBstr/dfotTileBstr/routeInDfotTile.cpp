#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/routeInDfotTile.h"
#include "log.h"

// #include <boost/foreach.hpp>

namespace BitGen {
namespace bitstream {

void contRoutesDfotTileBstr::checkSiteNameOfPip(vecBits &bits) {
#ifdef _DEBUG
  string grmName = _refTile->getTrnss().getGRMName();
  for (bitTile &bit : bits) {
    ASSERTD(bit._siteName == grmName,
            "siteName of PIP != Name of GRM in this tile ... " + bit._siteName +
                " != " + grmName);
  }
#endif
}

} // namespace bitstream
} // namespace BitGen