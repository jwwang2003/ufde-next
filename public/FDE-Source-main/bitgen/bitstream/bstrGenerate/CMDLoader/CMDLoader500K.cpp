#include "bitstream/bstrGenerate/CMDLoader/CMDLoader500K.h"

namespace BitGen {
namespace bitstream {
void CMDLoader500K::bstrGen(const std::string &outputFile) {
  bsHeader();
  adjustSYNC("AA99_5566");
  insertCMD("0000_0007", "reset CRC");
  setFRMLen(90);
  setOption("0A20_302D");
  shapeMask("0000_0040");
  appointCTL("0000_0040");
  insertCMD("0000_0009", "switch clock");
  insertCMD("0000_0001", "write config");

  writeNomalTiles();

  insertCMD("0000_0007", "");
  insertCMD("0000_0005", "");
  insertCMD("0000_000D", "");

  exportBitFile(outputFile);
}
} // namespace bitstream
} // namespace BitGen