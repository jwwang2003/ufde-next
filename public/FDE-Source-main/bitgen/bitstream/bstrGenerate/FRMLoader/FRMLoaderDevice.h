#ifndef _FRMLOADERDEVICE_H
#define _FRMLOADERDEVICE_H

#include "bitstream/bstrGenerate/FRMLoader/FRMLoaderBase.h"

namespace BitGen {
namespace bitstream {
class FRMLoader1000K : public FRMLoaderBase {
public:
  FRMLoader1000K(cktMemLibBstr *refCktMemLib = 0,
                 cktTileLibBstr *refCktTileLib = 0,
                 FDU::cil_lib::majorLib *refMajorLib = 0)
      : FRMLoaderBase(refCktMemLib, refCktTileLib, refMajorLib) {}

  void tellAddrFrm(int addr, const std::string &type);
};

class FRMLoader3000K : public FRMLoader1000K {
public:
  FRMLoader3000K(cktMemLibBstr *refCktMemLib = 0,
                 cktTileLibBstr *refCktTileLib = 0,
                 FDU::cil_lib::majorLib *refMajorLib = 0)
      : FRMLoader1000K(refCktMemLib, refCktTileLib, refMajorLib) {}
};

class FRMLoader80K : public FRMLoaderBase {
public:
  FRMLoader80K(cktMemLibBstr *refCktMemLib = 0,
               cktTileLibBstr *refCktTileLib = 0,
               FDU::cil_lib::majorLib *refMajorLib = 0)
      : FRMLoaderBase(refCktMemLib, refCktTileLib, refMajorLib) {}

  void tellAddrFrm(int addr, const std::string &type);
};

class FRMLoader500K : public FRMLoader80K {
public:
  FRMLoader500K(cktMemLibBstr *refCktMemLib = 0,
                cktTileLibBstr *refCktTileLib = 0,
                FDU::cil_lib::majorLib *refMajorLib = 0)
      : FRMLoader80K(refCktMemLib, refCktTileLib, refMajorLib) {}
};

inline void
FRMLoader1000K::tellAddrFrm(int addr,
                            const std::string &type) { // 6 bits for each addr
  tellAddrMajor(addr, type);
}

inline void
FRMLoader80K::tellAddrFrm(int addr,
                          const std::string &type) { // 6 bits for each addr
  _ofs << ((addr >> 6) & 0x01) << type << addr << "\n";
  for (int i = 5; i >= 0; --i)
    _ofs << ((addr >> i) & 0x01) << "\n";
}

} // namespace bitstream
} // namespace BitGen
#endif