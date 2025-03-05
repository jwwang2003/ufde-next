#ifndef _TILE_H_
#define _TILE_H_

#include "log.h"
#include "transInTile.h"

namespace FDU {
namespace cil_lib {

class Tile : public CilBase {
private:
  sizeSpan _sramSize;
  contClstsTile _clsts;
  contTrnssTile _trnss;
  clstLib *_refClstLib;
  trnsLib *_refTrnsLib;
  //		tileLib* _refTileLib;

public:
  Tile(const std::string &name, sizeSpan sramSize, clstLib *refClstLib = 0,
       trnsLib *refTrnsLib = 0)
      : CilBase(name), _sramSize(sramSize), _refClstLib(refClstLib),
        _refTrnsLib(refTrnsLib) {
    construct();
  }

  contClstsTile &getClsts() { return _clsts; }
  contTrnssTile &getTrnss() { return _trnss; }
  sizeSpan &getSramSize() { return _sramSize; }
  std::string getGRMName();

  void listDfotCfgBits(vecBits &bits);
  void listDfotCfgs(vecCfgs &cfgs);
  void listDfotPips(vecBits &bits);

  Site *getRefSite(const std::string &siteName);
  void getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement);
  void getPathPips(vecBits &bits, const routeInfo &route);
  sramInSiteSramTile *getRefSram(const bitTile &bit);
  void setBitPlace(bitTile &bit);

  void construct();
};

inline std::string Tile::getGRMName() { return _trnss.getGRMName(); }

inline void Tile::listDfotCfgBits(vecBits &bits) {
  _clsts.listDfotCfgBits(bits);
}

inline void Tile::listDfotCfgs(vecCfgs &cfgs) { _clsts.listDfotCfgs(cfgs); }

inline void Tile::listDfotPips(vecBits &bits) { _trnss.listDfotPips(bits); }

inline Site *Tile::getRefSite(const std::string &siteName) {
  Site *site = _clsts.getRefSite(siteName);
  if (!site)
    site = _trnss.getRefSite(siteName);
  ASSERT(site, "tile: no such site named ... " + siteName);
  return site;
}

inline sramInSiteSramTile *Tile::getRefSram(const bitTile &bit) {
  sramInSiteSramTile *sram = _clsts.getRefSram(bit);
  if (!sram)
    sram = _trnss.getRefSram(bit);
  ASSERT(sram, "tile: no such sram named ... " + bit._siteName + " " +
                   bit._basicCell + " " + bit._sramName);
  return sram;
}

} // namespace cil_lib
} // namespace FDU

#endif