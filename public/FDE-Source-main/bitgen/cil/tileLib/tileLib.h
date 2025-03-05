#ifndef _TILELIB_H_
#define _TILELIB_H_

#include "cil/tileLib/tile/tile.h"

namespace FDU {
namespace cil_lib {

class tileLib {
public:
  using tilesType = cilContainer<Tile>::range_type;
  using const_tilesType = cilContainer<Tile>::const_range_type;
  using tileIter = cilContainer<Tile>::iterator;
  using const_tileIter = cilContainer<Tile>::const_iterator;

private:
  cilContainer<Tile> _tiles;
  clstLib *_refClstLib;
  trnsLib *_refTrnsLib;

public:
  tileLib(clstLib *refClstLib = 0, trnsLib *refTrnsLib = 0)
      : _refClstLib(refClstLib), _refTrnsLib(refTrnsLib) {}

  tilesType tiles() { return _tiles.range(); }
  const_tilesType tiles() const { return _tiles.range(); }

  Tile *addTile(Tile *tile) { return _tiles.add(tile); }
  Tile &getTile(const std::string &tileName) { return *tiles().find(tileName); }

  void listDfotCfgs(vecCfgs &cfgs);
};

} // namespace cil_lib
} // namespace FDU

#endif