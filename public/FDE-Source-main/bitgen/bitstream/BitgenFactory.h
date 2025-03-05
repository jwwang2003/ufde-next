#ifndef _BITGENFACTORY_H_
#define _BITGENFACTORY_H_

#include "bitstream/bstrGenerate/CMDLoader/CMDLoader1000K.h"
#include "bitstream/bstrGenerate/CMDLoader/CMDLoader3000K.h"
#include "bitstream/bstrGenerate/CMDLoader/CMDLoader500K.h"
#include "bitstream/bstrGenerate/CMDLoader/CMDLoader80K.h"
#include "bitstream/bstrGenerate/FRMLoader/FRMLoaderDevice.h"

namespace BitGen {
namespace bitstream {

class BitgenFactory {
public:
  using pointer = std::auto_ptr<BitgenFactory>;
  static BitgenFactory &instance() { return *_instance.get(); }
  static pointer set_factory(pointer f) {
    pointer p = _instance;
    _instance = f;
    return p;
  }
  static pointer set_factory(BitgenFactory *f) {
    return set_factory(pointer(f));
  }

  virtual CMDLoaderBase *
  make_cmdloader(cktMemLibBstr *refCktMemLib, cktTileLibBstr *refCktTileLib,
                 FDU::cil_lib::majorLib *refMajorLib) = 0;

  virtual FRMLoaderBase *
  make_frmloader(cktMemLibBstr *refCktMemLib, cktTileLibBstr *refCktTileLib,
                 FDU::cil_lib::majorLib *refMajorLib) = 0;

private:
  static pointer _instance;
};

class BitgenFactory3000K : public BitgenFactory {
  CMDLoaderBase *make_cmdloader(cktMemLibBstr *refCktMemLib,
                                cktTileLibBstr *refCktTileLib,
                                FDU::cil_lib::majorLib *refMajorLib);
  virtual FRMLoaderBase *make_frmloader(cktMemLibBstr *refCktMemLib,
                                        cktTileLibBstr *refCktTileLib,
                                        FDU::cil_lib::majorLib *refMajorLib);
};

class BitgenFactory1000K : public BitgenFactory {
  CMDLoaderBase *make_cmdloader(cktMemLibBstr *refCktMemLib,
                                cktTileLibBstr *refCktTileLib,
                                FDU::cil_lib::majorLib *refMajorLib);
  virtual FRMLoaderBase *make_frmloader(cktMemLibBstr *refCktMemLib,
                                        cktTileLibBstr *refCktTileLib,
                                        FDU::cil_lib::majorLib *refMajorLib);
};

class BitgenFactory80K : public BitgenFactory {
  CMDLoaderBase *make_cmdloader(cktMemLibBstr *refCktMemLib,
                                cktTileLibBstr *refCktTileLib,
                                FDU::cil_lib::majorLib *refMajorLib);
  virtual FRMLoaderBase *make_frmloader(cktMemLibBstr *refCktMemLib,
                                        cktTileLibBstr *refCktTileLib,
                                        FDU::cil_lib::majorLib *refMajorLib);
};

class BitgenFactory500K : public BitgenFactory {
  CMDLoaderBase *make_cmdloader(cktMemLibBstr *refCktMemLib,
                                cktTileLibBstr *refCktTileLib,
                                FDU::cil_lib::majorLib *refMajorLib);
  virtual FRMLoaderBase *make_frmloader(cktMemLibBstr *refCktMemLib,
                                        cktTileLibBstr *refCktTileLib,
                                        FDU::cil_lib::majorLib *refMajorLib);
};

} // namespace bitstream
} // namespace BitGen
#endif