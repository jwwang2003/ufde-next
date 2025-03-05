#ifndef _CMDLOADER80K_H_
#define _CMDLOADER80K_H_

#include "bitstream/bstrGenerate/CMDLoader/CMDLoaderBase.h"

namespace BitGen {
namespace bitstream {

class CMDLoader : public CMDLoaderBase {
public:
  CMDLoader(cktMemLibBstr *refCktMemLib = 0, cktTileLibBstr *refCktTileLib = 0,
            FDU::cil_lib::majorLib *refMajorLib = 0,
            FDU::cil_lib::bstrCMDLib *refbstrCMDLib = 0)
      : CMDLoaderBase(refCktMemLib, refCktTileLib, refMajorLib, refbstrCMDLib) {
  }

  void bstrGen(const std::string &outputFile);
};
} // namespace bitstream
} // namespace BitGen
#endif