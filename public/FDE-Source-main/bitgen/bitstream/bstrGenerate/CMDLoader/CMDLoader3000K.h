#ifndef _CMDLOADER3000K_H_
#define _CMDLOADER3000K_H_

#include "bitstream/bstrGenerate/CMDLoader/CMDLoader1000K.h"

namespace BitGen {
namespace bitstream {

class CMDLoader3000K : public CMDLoader1000K {
private:
  void dummyFor3000K();

public:
  CMDLoader3000K(cktMemLibBstr *refCktMemLib = 0,
                 cktTileLibBstr *refCktTileLib = 0,
                 FDU::cil_lib::majorLib *refMajorLib = 0)
      : CMDLoader1000K(refCktMemLib, refCktTileLib, refMajorLib) {}

  void bstrGen(const std::string &outputFile);
};
} // namespace bitstream
} // namespace BitGen
#endif