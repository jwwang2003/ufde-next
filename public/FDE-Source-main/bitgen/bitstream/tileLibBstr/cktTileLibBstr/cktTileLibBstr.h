#ifndef _CKTTILELIBBSTR_H_
#define _CKTTILELIBBSTR_H_

#include "bitstream/memLibBstr/cktMemLibBstr/cktMemLibBstr.h"
#include "bitstream/tileLibBstr/cktTileLibBstr/cktTileBstr/cktTileBstr.h"
#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileLibBstr.h"
#include "cil/cilLib.h"
#include "circuit/circuit.h"
#include "log.h"

#include <boost/lexical_cast.hpp>

namespace BitGen {
namespace bitstream {

class cktTileLibBstr {
public:
  using cktTilesType = cktContainer<cktTileBstr>::range_type;
  using const_cktTilesType = cktContainer<cktTileBstr>::const_range_type;
  using cktTileIter = cktContainer<cktTileBstr>::iterator;
  using const_cktTileIter = cktContainer<cktTileBstr>::const_iterator;

private:
  cktContainer<cktTileBstr> _cktTiles;
  sizeSpan _fpgaScale;
  BitGen::circuit::Circuit *_refCkt;
  FDU::cil_lib::cilLibrary *_refCil;
  BitGen::bitstream::dfotTileLibBstr *_refDfotTileLib;
  BitGen::bitstream::cktMemLibBstr *_refCktMemLib;

public:
  cktTileLibBstr(FDU::cil_lib::cilLibrary *refCil = 0,
                 BitGen::circuit::Circuit *refCkt = 0,
                 BitGen::bitstream::dfotTileLibBstr *refDfotTileLib = 0,
                 BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0)
      : _refCil(refCil), _refCkt(refCkt), _refDfotTileLib(refDfotTileLib),
        _refCktMemLib(refCktMemLib) {}

  cktTilesType cktTiles() { return _cktTiles.range(); }
  const_cktTilesType cktTiles() const { return _cktTiles.range(); }

  cktTileBstr *addTile(cktTileBstr *tile) { return _cktTiles.add(tile); }
  cktTileBstr &getTile(const std::string &tileName);
  cktTileBstr &getTile(const sizeSpan &tilePos);

  sizeSpan getFRMBits(std::vector<int> &FRMBits, int tileCol, int FRM);
  sizeSpan getColBits(std::vector<int> &colBits, int tileCol);

  void construct(const std::string &device);
  void initialize(const std::string &device);
  void recordCktCfgs();
  void recordCktCfgs1000K();
  void recordCktCfgsFDP80K();
  void recordCktPips();
  void analyze();
  void regulate();
  void build();
  void checkOverlaps();
};

using namespace boost;

inline cktTileBstr &cktTileLibBstr::getTile(const std::string &tileName) {
  cktTileIter it = find_if(cktTiles(), [&tileName](const cktTileBstr *tile) {
    return tile->getName() == tileName;
  });
  ASSERTD(it != _cktTiles.end(),
          "cktTileLib: no such tile named ... " + tileName);
  return **it;
}

inline cktTileBstr &cktTileLibBstr::getTile(const sizeSpan &tilePos) {
  int rows = _fpgaScale._rowSpan, cols = _fpgaScale._columnSpan;
  ASSERTD((tilePos._rowSpan >= 0 && tilePos._rowSpan < rows) &&
              (tilePos._columnSpan >= 0 && tilePos._columnSpan < cols),
          "cktTileLib: no such tile located ... " +
              boost::lexical_cast<string>(tilePos));
  return *_cktTiles.at(tilePos._rowSpan * cols + tilePos._columnSpan);
}

inline sizeSpan cktTileLibBstr::getFRMBits(std::vector<int> &FRMBits,
                                           int tileCol, int FRM) {
  int rows = _fpgaScale._rowSpan, cols = _fpgaScale._columnSpan,
      FRMBitsAmnt = 0;
  ASSERTD(tileCol >= 0 && tileCol < cols,
          "cktTileLib: col of tile out of range");
  for (int i = 0; i < rows; ++i)
    FRMBitsAmnt += getTile(sizeSpan(i, tileCol)).getFRMBits(FRMBits, FRM);
  return sizeSpan(FRMBitsAmnt, 1);
}

} // namespace bitstream
} // namespace BitGen

#endif