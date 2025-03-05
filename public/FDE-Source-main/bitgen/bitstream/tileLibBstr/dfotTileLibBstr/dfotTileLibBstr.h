#ifndef _DFOTTILELIBBSTR_H_
#define _DFOTTILELIBBSTR_H_

#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/dfotTileBstr.h"
#include "cil/tileLib/tileLib.h"
#include "container/Container.h"
#include "utils/cfgInTile.h"

namespace BitGen {
namespace bitstream {

class dfotTileLibBstr {
public:
  using dfotTilesType = bstrContainer<dfotTileBstr>::range_type;
  using const_dfotTilesType = bstrContainer<dfotTileBstr>::const_range_type;
  using dfotTileIter = bstrContainer<dfotTileBstr>::iterator;
  using const_dfotTileIter = bstrContainer<dfotTileBstr>::const_iterator;

private:
  bstrContainer<dfotTileBstr> _dfotTiles;
  FDU::cil_lib::tileLib *_refTileLib;

public:
  explicit dfotTileLibBstr(FDU::cil_lib::tileLib *refTileLib = 0)
      : _refTileLib(refTileLib) {}

  //		void setRefTileLib(FDU::cil_lib::tileLib* ref) { _refTileLib =
  // ref; }

  dfotTilesType dfotTiles() { return _dfotTiles.range(); }
  const_dfotTilesType dfotTiles() const { return _dfotTiles.range(); }
  dfotTileBstr *addDfotTile(dfotTileBstr *tile) { return _dfotTiles.add(tile); }
  dfotTileBstr &getDfotTile(const std::string &tileName) {
    return *dfotTiles().find(tileName);
  }

  void construct();
  void initialize();
  void listDfotCfgs();
  void analyze();
  void regulate();
  void build();
  void checkOverlaps();
};

} // namespace bitstream
} // namespace BitGen

#endif