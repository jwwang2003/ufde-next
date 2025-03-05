#include "bitstream/bstrGenerate/CMDLoader/CMDLoader80K.h"

namespace BitGen {
namespace bitstream {

void CMDLoader80K::mountSMFA(int majorAddr) {
  _ofs << "3000_2001"
       << "\n";
  wrdToOstream(0, majorAddr * 0x80000);
}

void CMDLoader80K::bstrGen(const std::string &outputFile) {
  bsHeader();
  adjustSYNC("AA99_5566");
  insertCMD("0000_0007", "reset CRC");
  setFRMLen(50);
  setOption("0A00_382D");
  shapeMask("0000_01C1");
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