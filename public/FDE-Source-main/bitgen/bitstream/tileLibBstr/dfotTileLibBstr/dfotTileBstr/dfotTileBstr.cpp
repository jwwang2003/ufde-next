#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/dfotTileBstr.h"

#include <iomanip>
#include <iostream>

namespace BitGen {
namespace bitstream {
using namespace std;

void dfotTileBstr::construct() {
  if (_refTile) {
    _logics.setRefTile(_refTile);
    _routes.setRefTile(_refTile);

    _name = _refTile->getName();
    _bstrSize = _refTile->getSramSize();
  }
}

void dfotTileBstr::analyzeBits() {
  vecBits &elementLogicBits = _tileBits._bits;
  _logics.listDfotCfgBits(elementLogicBits);
  _logics.analyzeLogicPlace(elementLogicBits);

  vecBits siteLogicBits;
  _logics.analyzeLogicFunction(siteLogicBits);
  _logics.analyzeLogicPlace(siteLogicBits);
  _tileBits.appendBits(siteLogicBits);

  vecBits routeBits;
  _routes.listDfotPips(routeBits);
  _routes.analyzePipPlace(routeBits);
  _tileBits.appendBits(routeBits);
}

void dfotTileBstr::regulateBits(vecBits &lodgerBits) {
  if (_tileBits.existLodgerBits()) {
    vecBits ownBits;
    for (const bitTile &bit : _tileBits._bits)
      if (bit.isLodger())
        lodgerBits.push_back(bit);
      else
        ownBits.push_back(bit);
    _tileBits.resetBits(ownBits);
  }
}

void dfotTileBstr::buildBitArry() {
  int rowSpan = _bstrSize._rowSpan, colSpan = _bstrSize._columnSpan;
  if (args._device == CHIPTYPE::FDP500KIP && _name == "BRAMSITE") {
    _tileBstr.resize(rowSpan * colSpan, 0);
  } else {
    _tileBstr.resize(rowSpan * colSpan, 1);
  }
  for (bitTile &bit : _tileBits._bits) {
    int row = bit._localPlace._rowSpan, col = bit._localPlace._columnSpan;
    _tileBstr[row * colSpan + col] = bit._bitContent;
  }
}

void dfotTileBstr::exportArry(const string &file) const {
  ofstream arry(file.c_str());
  for (int row = 0; row < _bstrSize._rowSpan; ++row) {
    for (int col = 0; col < _bstrSize._columnSpan; ++col) {
      arry << setw(2) << _tileBstr.at(row * _bstrSize._columnSpan + col);
    }
    arry << endl;
  }
}

bool dfotTileBstr::checkOverlaps() {
  _hasOvlps = false;
  int rowSpan = _bstrSize._rowSpan, colSpan = _bstrSize._columnSpan;
  _overlaps.resize(rowSpan * colSpan);
  for (bitTile &bit : _tileBits._bits) {
    int row = bit._localPlace._rowSpan, col = bit._localPlace._columnSpan;
    int target = row * colSpan + col;
    _overlaps[target].push_back(&bit);
    if (_overlaps[target].size() > 1)
      _hasOvlps = true;
  }
  _overlaps.clear();
  return _hasOvlps;
}

} // namespace bitstream
} // namespace BitGen