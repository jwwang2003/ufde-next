#include "bitstream/bstrGenerate/CMDLoader/CMDLoader1000K.h"
#include "main/arguments/Args.h"

namespace BitGen {
namespace bitstream {

void CMDLoader1000K::mountSMFA(int majorAddr) {
  _ofs << "3000_2001"
       << "\n";
  wrdToOstream(0, majorAddr * 0x20000);
}

void CMDLoader1000K::bstrGen(const std::string &outputFile) {
  bsHeader();
  adjustSYNC("AA99_5566");
  insertCMD("0000_0007", "reset CRC");
  setFRMLen(12);
  setOption("0BC0_382D");
  shapeMask("0000_01C1");
  insertCMD("0000_0009", "switch clock");
  insertCMD("0000_0001", "write config");

  writeNomalTiles();

  int memAmount = cktMemLibBstr::_1000K_MEM_AMOUNT;
  dfotMemBstr::BitList memBitList;
  for (int memIdx = 0; memIdx < memAmount; ++memIdx) {
    _refCktMemLib->getMemContents(memBitList, memIdx);
    int wrdsAmnt = memBitList.size();
    mountBMFA(memIdx + 1);
    mapOutFDRI(wrdsAmnt + 1);
    memToOstream(memBitList);
    fillBlank(1);
    memBitList.clear();
  }

  insertCMD("0000_0005", "start up");
  appointCTL("0000_0040");

  exportBitFile(outputFile);
}

} // namespace bitstream
} // namespace BitGen