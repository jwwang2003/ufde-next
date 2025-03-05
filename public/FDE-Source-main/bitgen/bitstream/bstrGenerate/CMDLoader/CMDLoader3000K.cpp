#include "bitstream/bstrGenerate/CMDLoader/CMDLoader3000K.h"
#include "main/arguments/Args.h"

namespace BitGen {
namespace bitstream {

void CMDLoader3000K::dummyFor3000K() {
  _ofs << "3000_2001"
       << "\n"
       << "0000_0000"
       << "\n";
}

void CMDLoader3000K::bstrGen(const std::string &outputFile) {
  bsHeader();
  adjustSYNC("AA99_5566");
  insertCMD("0000_0007", "reset CRC");
  setFRMLen(19);
  setOption("0A01_302D");
  shapeMask("0000_01C1");
  insertCMD("0000_0009", "switch clock");
  dummyFor3000K();
  insertCMD("0000_0001", "write config");

  writeNomalTiles();

  int memAmount = cktMemLibBstr::_3000K_MEM_AMOUNT;
  dfotMemBstr::BitList memBitList;
  for (int memIdx = 0; memIdx < memAmount; ++memIdx) {
    _refCktMemLib->getMemContents(memBitList, memIdx);
    int wrdsAmnt = memBitList.size();
    mountBMFA(memIdx + 1);
    mapOutFDRI(wrdsAmnt);
    memToOstream(memBitList);
    memBitList.clear();
  }

  insertCMD("0000_0007", "");
  insertCMD("0000_0005", "");
  insertCMD("0000_000D", "");

  exportBitFile(outputFile);
}
} // namespace bitstream
} // namespace BitGen