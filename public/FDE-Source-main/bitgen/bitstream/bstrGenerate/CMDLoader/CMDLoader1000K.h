#ifndef _CMDLOADER1000K_H_
#define _CMDLOADER1000K_H_

#include "bitstream/bstrGenerate/CMDLoader/CMDLoaderBase.h"

namespace BitGen {
namespace bitstream {

class CMDLoader1000K : public CMDLoaderBase {
protected:
  void reverse(std::vector<int> &FRMBits);
  void mountSMFA(int majorAddr);

public:
  CMDLoader1000K(cktMemLibBstr *refCktMemLib = 0,
                 cktTileLibBstr *refCktTileLib = 0,
                 FDU::cil_lib::majorLib *refMajorLib = 0)
      : CMDLoaderBase(refCktMemLib, refCktTileLib, refMajorLib) {}

  void bstrGen(const std::string &outputFile);
};

inline void CMDLoader1000K::reverse(std::vector<int> &FRMBits) {
  for (int i = 0, size = FRMBits.size(); i < size;
       i += _BITS_PER_GRP_REVERSED_3000K)
    for (int j = i, end = j + _BITS_PER_GRP_REVERSED_3000K - 1; j < end;
         ++j, --end)
      std::swap(FRMBits[j], FRMBits[end]);
}

} // namespace bitstream
} // namespace BitGen
#endif