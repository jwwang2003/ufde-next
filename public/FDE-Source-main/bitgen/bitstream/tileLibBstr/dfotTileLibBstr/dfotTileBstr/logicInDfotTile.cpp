#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/logicInDfotTile.h"

namespace BitGen {
namespace bitstream {

void contLogicsDfotTileBstr::listDfotCfgBits(vecBits &bits) {
  _refTile->listDfotCfgBits(bits);
}

void contLogicsDfotTileBstr::analyzeLogicFunction(vecBits &bits) {
  vecBits tBits;
  for (cfgElem &logic : _logics) {
    _refTile->getContentsOfFunc(tBits, logic);
    for (bitTile &bit : tBits) {
      bit._type = bitTile::logic;
      bit._siteName = logic._siteName;
      bit._cfgElemName = logic._cfgElemName;
      bit._cfgElemFunc = logic._cfgElemFunc;
      bits.push_back(bit);
    }
    tBits.clear();
  }
}

void contLogicsDfotTileBstr::analyzeLogicPlace(vecBits &bits) {
  for (bitTile &bit : bits)
    _refTile->setBitPlace(bit);
}

} // namespace bitstream
} // namespace BitGen