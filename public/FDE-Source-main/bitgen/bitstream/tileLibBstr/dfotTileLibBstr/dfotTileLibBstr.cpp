#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileLibBstr.h"
#include "cil/tileLib/tile/tile.h"
#include "log.h"
#include "utils.h"

#include <boost/range/adaptors.hpp>
#include <iostream>

namespace BitGen {
namespace bitstream {
using namespace FDU::cil_lib;
using namespace boost::adaptors;
using namespace boost;

void dfotTileLibBstr::construct() {
  initialize();
  listDfotCfgs();
  analyze();
  regulate();
  build();
  checkOverlaps();
}

void dfotTileLibBstr::initialize() {
  for (Tile *tile : _refTileLib->tiles()) {
    /*dfotTileBstr& dfotTile = */ addDfotTile(new dfotTileBstr(tile));
  }
}

void dfotTileLibBstr::listDfotCfgs() {
  vecCfgs cfgs;
  _refTileLib->listDfotCfgs(cfgs);
  for (cfgElem &cfg : cfgs) {
    getDfotTile(cfg._tileName).addLogic(cfg);
  }
}

void dfotTileLibBstr::analyze() {
  for (dfotTileBstr *dfotTile : dfotTiles())
    dfotTile->analyzeBits();
}

void dfotTileLibBstr::regulate() {
  vecBits lodgerBits;
  for (dfotTileBstr *tile : dfotTiles()) {
    tile->regulateBits(lodgerBits);
    for (bitTile &bit : lodgerBits) {
      dfotTileIter it =
          find_if(dfotTiles(), [&bit](const dfotTileBstr *dfotTile) {
            return dfotTile->getName() == bit._ownerTile;
          });
      ASSERTD(it != _dfotTiles.end(),
              "lodger bit: does not exist my owner Tile ... " + bit._ownerTile);
      bit._landLordTile = bit._ownerTile;
      it->addBit(bit);
    }
    lodgerBits.clear();
  }
}

void dfotTileLibBstr::build() {
  for (dfotTileBstr *dfotTile : dfotTiles())
    dfotTile->buildBitArry();
}

void dfotTileLibBstr::checkOverlaps() {
  for (dfotTileBstr *dfotTile : dfotTiles())
    if (dfotTile->checkOverlaps())
      FDU_LOG(WARN) << Warning("bitgen: overlap bits exist at default tile - " +
                               dfotTile->getName());
}

} // namespace bitstream
} // namespace BitGen