#ifndef _LOADERBASE_H_
#define _LOADERBASE_H_

#include "bitstream/memLibBstr/cktMemLibBstr/cktMemLibBstr.h"
#include "bitstream/tileLibBstr/cktTileLibBstr/cktTileLibBstr.h"
#include "cil/bstrCMDLib/bstrCMDLib.h"
#include "cil/majorLib/majorLib.h"

namespace BitGen {
namespace bitstream {

class loaderBase {
protected:
  cktMemLibBstr *_refCktMemLib;
  cktTileLibBstr *_refCktTileLib;
  FDU::cil_lib::majorLib *_refMajorLib;
  FDU::cil_lib::bstrCMDLib *_refbstrCMDLib;

public:
  loaderBase(cktMemLibBstr *refCktMemLib = 0, cktTileLibBstr *refCktTileLib = 0,
             FDU::cil_lib::majorLib *refMajorLib = 0,
             FDU::cil_lib::bstrCMDLib *refbstrCMDLib = 0)
      : _refCktMemLib(refCktMemLib), _refCktTileLib(refCktTileLib),
        _refMajorLib(refMajorLib), _refbstrCMDLib(refbstrCMDLib) {}
  virtual ~loaderBase() {}

  virtual void bstrGen(const std::string &outputFile) = 0;
};

} // namespace bitstream
} // namespace BitGen

#endif