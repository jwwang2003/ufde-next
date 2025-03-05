#ifndef _LOGICINDFOTTILE_H_
#define _LOGICINDFOTTILE_H_

#include "cil/tileLib/tile/tile.h"
#include "utils/cfgInTile.h"

#include <string>
#include <vector>

namespace BitGen {
namespace bitstream {

class contLogicsDfotTileBstr {
public:
  using Logics = std::vector<cfgElem>;

private:
  Logics _logics;
  FDU::cil_lib::Tile *_refTile;

public:
  void setRefTile(FDU::cil_lib::Tile *ref) { _refTile = ref; }

  cfgElem &addLogic(const cfgElem &logic) {
    _logics.push_back(logic);
    return _logics.back();
  }
  Logics &getLogics() { return _logics; }

  void listDfotCfgBits(vecBits &bits);

  void analyzeLogicFunction(vecBits &bits);
  void analyzeLogicPlace(vecBits &bits);
};

} // namespace bitstream
} // namespace BitGen

#endif