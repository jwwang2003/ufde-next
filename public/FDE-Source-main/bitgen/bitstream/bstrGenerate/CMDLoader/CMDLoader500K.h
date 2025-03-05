#ifndef _CMDLOADER500K_H
#define _CMDLOADER500K_H

#include "bitstream/bstrGenerate/CMDLoader/CMDLoader80K.h"

namespace BitGen {
namespace bitstream {

class CMDLoader500K : public CMDLoader80K {
protected:
  int adjust(std::vector<int> &FRMBits) {
    return CMDLoaderBase::adjust(FRMBits);
  };

public:
  CMDLoader500K(cktMemLibBstr *refCktMemLib = 0,
                cktTileLibBstr *refCktTileLib = 0,
                FDU::cil_lib::majorLib *refMajorLib = 0)
      : CMDLoader80K(refCktMemLib, refCktTileLib, refMajorLib) {}
  void bstrGen(const std::string &outputFile);
};
} // namespace bitstream
} // namespace BitGen
#endif