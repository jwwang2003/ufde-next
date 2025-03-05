#ifndef _DFOTTILEBSTR_H_
#define _DFOTTILEBSTR_H_

#include "PropertyName.h"
#include "bitstream/bstrBase.h"
#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/bitInDfotTile.h"
#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/logicInDfotTile.h"
#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/routeInDfotTile.h"
#include "log.h"
#include "main/arguments/Args.h"

namespace BitGen {
namespace bitstream {

using namespace FDU;

class dfotTileBstr : public bstrBase {
public:
  using tileBstr = std::vector<int>;
  using Overlaps = std::vector<std::vector<bitTile *>>;

protected:
  bool _hasOvlps;
  contLogicsDfotTileBstr _logics;
  contRoutesDfotTileBstr _routes;
  contBitsDfotTileBstr _tileBits;
  tileBstr _tileBstr;
  Overlaps _overlaps;
  sizeSpan _bstrSize;
  FDU::cil_lib::Tile *_refTile;

public:
  explicit dfotTileBstr(FDU::cil_lib::Tile *refTile = 0)
      : bstrBase(""), _refTile(refTile), _hasOvlps(false) {
    construct();
  }

  cfgElem &addLogic(const cfgElem &logic) { return _logics.addLogic(logic); }
  contLogicsDfotTileBstr &getLogics() { return _logics; }

  routeInfo &addRoute(const routeInfo &route) {
    return _routes.addRoute(route);
  }
  contRoutesDfotTileBstr &getRoutes() { return _routes; }

  const vecBits &getBits() const { return _tileBits._bits; }
  bitTile &addBit(const bitTile &bit) { return _tileBits.addBit(bit); }
  tileBstr &getTileBstr() { return _tileBstr; }
  const Overlaps &getOverlaps() const { return _overlaps; }
  sizeSpan getBstrSize() const { return _bstrSize; }
  bool hasOverlaps() const { return _hasOvlps; }
  bool checkOverlaps();

  FDU::cil_lib::Tile *getRefTile() const { return _refTile; }

  virtual int getFRMBits(std::vector<int> &FRMBits, int FRM);

  virtual void analyzeBits();
  virtual void regulateBits(vecBits &lodgerBits);
  virtual void buildBitArry();
  virtual void exportArry(const string &file) const;

  virtual void construct();
};

inline int dfotTileBstr::getFRMBits(std::vector<int> &FRMBits, int FRM) {
  int rows = _bstrSize._rowSpan, cols = _bstrSize._columnSpan;
  ASSERTD(FRM >= 0 && FRM < cols, "tile: FRM index out of range");
  for (int i = 0; i < rows; ++i)
    FRMBits.push_back(_tileBstr.at(i * cols + FRM));
  if (args._device == CHIPTYPE::FDP80K) {
    if (rows == 48) // convenient for hardware decoder.
      for (int j = rows; j < 80; ++j)
        FRMBits.push_back(0);
    return 80;
  } else if (args._device == CHIPTYPE::FDP500K ||
             args._device == CHIPTYPE::FDP500KIP) {
    if (rows == 48)
      for (int j = rows; j < 80; ++j)
        FRMBits.push_back(1);
    return 80;
  }
  return rows;
}

} // namespace bitstream
} // namespace BitGen

#endif