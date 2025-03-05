#ifndef _FRMLOADERBASE_H_
#define _FRMLOADERBASE_H_

#include "PropertyName.h"
#include "bitstream/bstrGenerate/loaderBase.h"
#include "main/arguments/Args.h"

#include <fstream>

namespace BitGen {
namespace bitstream {
using namespace FDU;
class FRMLoaderBase : public loaderBase {
public:
  static const string _MAJOR_ADDR;
  static const string _FRAME_ADDR;

protected:
  std::ofstream _ofs;

protected:
  void tellAddr(int addr, const std::string &type, int bits);
  void bitsToOstream(const std::vector<int> &colBits, const sizeSpan &bitSize,
                     int majorAddr);

public:
  FRMLoaderBase(cktMemLibBstr *refCktMemLib = 0,
                cktTileLibBstr *refCktTileLib = 0,
                FDU::cil_lib::majorLib *refMajorLib = 0,
                FDU::cil_lib::bstrCMDLib *refbstrCMDLib = 0)
      : loaderBase(refCktMemLib, refCktTileLib, refMajorLib, refbstrCMDLib) {}
  void bstrGen(const std::string &outputFile);
};

inline void FRMLoaderBase::tellAddr(int addr, const std::string &type,
                                    int bits) { // 6 bits for each addr
  _ofs << ((addr >> (--bits)) & 0x01) << type << addr << "\n";
  for (int i = (--bits); i >= 0; --i)
    _ofs << ((addr >> i) & 0x01) << "\n";
}

inline void FRMLoaderBase::bitsToOstream(const std::vector<int> &colBits,
                                         const sizeSpan &bitSize,
                                         int majorAddr) {
  int major_addr_bits =
      _refbstrCMDLib->getbstrParameter("major_addr_bits").getParameterValue();
  int frame_addr_bits =
      _refbstrCMDLib->getbstrParameter("frame_addr_bits").getParameterValue();
  int bitRowAmnt = bitSize._rowSpan, bitColAmnt = bitSize._columnSpan,
      bitIdx = 0;
  for (int col = 0; col < bitColAmnt; ++col) { // frames
    for (int row = 0; row < bitRowAmnt; ++row, ++bitIdx)
      _ofs << colBits[bitIdx] << "\n";
    tellAddr(majorAddr, _MAJOR_ADDR, major_addr_bits);
    tellAddr(col, _FRAME_ADDR, frame_addr_bits);
  }
}

} // namespace bitstream
} // namespace BitGen

#endif