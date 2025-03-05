#ifndef _CMDLOADER80K_H_
#define _CMDLOADER80K_H_

#include "bitstream/bstrGenerate/CMDLoader/CMDLoaderBase.h"

namespace BitGen {
namespace bitstream {

class CMDLoader80K : public CMDLoaderBase {
protected:
  int adjust(std::vector<int> &FRMBits);
  void reverse(std::vector<int> &FRMBits);
  void mountSMFA(int majorAddr);

public:
  CMDLoader80K(cktMemLibBstr *refCktMemLib = 0,
               cktTileLibBstr *refCktTileLib = 0,
               FDU::cil_lib::majorLib *refMajorLib = 0)
      : CMDLoaderBase(refCktMemLib, refCktTileLib, refMajorLib) {}

  void bstrGen(const std::string &outputFile);
};

inline int CMDLoader80K::adjust(std::vector<int> &FRMBits) {
  int bitsAmnt = FRMBits.size(), wordsAmnt = 0;
  if (bitsAmnt % _BITS_PER_WORD == 0) {
    wordsAmnt = bitsAmnt / _BITS_PER_WORD;
  } else {
    wordsAmnt = bitsAmnt / _BITS_PER_WORD + 1;
    FRMBits.resize(wordsAmnt * _BITS_PER_WORD, 0); // may change back to 1
  }
  return wordsAmnt;
}

inline void CMDLoader80K::reverse(std::vector<int> &FRMBits) {
  for (int i = 0, size = FRMBits.size(); i < size;
       i += _BITS_PER_GRP_REVERSED_80K)
    for (int j = i, end = j + _BITS_PER_GRP_REVERSED_80K - 1; j < end;
         ++j, --end)
      std::swap(FRMBits[j], FRMBits[end]);
}

} // namespace bitstream
} // namespace BitGen
#endif