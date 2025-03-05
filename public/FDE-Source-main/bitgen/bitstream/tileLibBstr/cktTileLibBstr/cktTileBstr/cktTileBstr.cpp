#include "bitstream/tileLibBstr/cktTileLibBstr/cktTileBstr/cktTileBstr.h"

#include "PropertyName.h"
#include "log.h"
#include "main/arguments/Args.h"

namespace BitGen {
namespace bitstream {
using namespace FDU;

void cktTileBstr::construct() {
  _used = false;
  if (_refDfotTile) {
    _refTile = _refDfotTile->getRefTile();
    ASSERTD(_refTile, "cktTile: my _refTile is nullptr");
    dfotTileBstr::construct();
  }
}

void cktTileBstr::analyzeBits() {
  vecBits &bits = _tileBits._bits;
  _logics.analyzeLogicFunction(bits);
  _logics.analyzeLogicPlace(bits);

  vecBits routeBits;
  _routes.listCktPips(routeBits);
  _routes.analyzePipPlace(routeBits);

  _tileBits.appendBits(routeBits);
}

void cktTileBstr::regulateBits(vecBits &lodgerBits) {
  if (_used && _tileBits.hasOffsetBits()) {
    vecBits ownBits;
    for (const bitTile &bit : _tileBits._bits)
      if (bit.hasOffset())
        lodgerBits.push_back(bit);
      else
        ownBits.push_back(bit);
    _tileBits.resetBits(ownBits);
    _used = _tileBits.isEmpty() ? false : _used;
    if (_bstrSize._rowSpan == 320)
      _used = true;
  }
}

void cktTileBstr::buildBitArry() {
  if (_used && _tileBstr.empty()) {
    int rows = _bstrSize._rowSpan, cols = _bstrSize._columnSpan;
    _tileBstr = _refDfotTile->getTileBstr();
    for (bitTile &bit : _tileBits._bits) {
      sizeSpan localPlace = bit._localPlace;
      int targetBit = localPlace._rowSpan * cols + localPlace._columnSpan;
      _tileBstr[targetBit] = bit._bitContent;
    }
  } else {
    if (args._device == CHIPTYPE::FDP1000K ||
        args._device == CHIPTYPE::FDP3000K)
      _tileBstr.resize(1, 1);
  }
}

} // namespace bitstream
} // namespace BitGen