#include "circuit/instLib/inst/posInInst.h"
#include "log.h"
#include "utils/sizeSpan.h"

namespace BitGen {
namespace circuit {

void posInst::constructFromXDL(const std::string &spot, const std::string &tile,
                               const std::string &site) {
  _spot = spot;
  _tile = tile;
  _site = site;

  format();
}

void posInst::format() {
  std::string::size_type dotPos = _site.find_last_of(".");
  if (dotPos != std::string::npos) {
    if (_site.substr(0, 4) == "TBUF") {
      ASSERT(dotPos + 2 == _site.length(), "TBUF: invalid site format in xdl");
      if (_site[dotPos - 1] % 2 == 0)
        _site[dotPos + 1] ^= 1;
    }
    _site.erase(0, dotPos + 1);
  }

  if ( // for fdp3000k-pq208
      _tile.substr(0, 4) == "BRAM" ||
      // for fdp3000k-cb228
      _tile.substr(0, 5) == "LBRAM" || _tile.substr(0, 5) == "RBRAM") {
    _site = "BRAM";
  }
}

} // namespace circuit
} // namespace BitGen