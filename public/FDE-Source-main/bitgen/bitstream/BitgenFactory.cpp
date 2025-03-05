#include "bitstream/BitgenFactory.h"

namespace BitGen {
namespace bitstream {

std::auto_ptr<BitgenFactory> BitgenFactory::_instance;

CMDLoaderBase *BitgenFactory3000K::make_cmdloader(
    BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0,
    BitGen::bitstream::cktTileLibBstr *refCktTileLib = 0,
    FDU::cil_lib::majorLib *refMajorLib = 0) {
  return new CMDLoader3000K(refCktMemLib, refCktTileLib, refMajorLib);
}

CMDLoaderBase *BitgenFactory1000K::make_cmdloader(
    BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0,
    BitGen::bitstream::cktTileLibBstr *refCktTileLib = 0,
    FDU::cil_lib::majorLib *refMajorLib = 0) {
  return new CMDLoader1000K(refCktMemLib, refCktTileLib, refMajorLib);
}

CMDLoaderBase *BitgenFactory80K::make_cmdloader(
    BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0,
    BitGen::bitstream::cktTileLibBstr *refCktTileLib = 0,
    FDU::cil_lib::majorLib *refMajorLib = 0) {
  return new CMDLoader80K(refCktMemLib, refCktTileLib, refMajorLib);
}

CMDLoaderBase *BitgenFactory500K::make_cmdloader(
    BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0,
    BitGen::bitstream::cktTileLibBstr *refCktTileLib = 0,
    FDU::cil_lib::majorLib *refMajorLib = 0) {
  return new CMDLoader500K(refCktMemLib, refCktTileLib, refMajorLib);
}

FRMLoaderBase *BitgenFactory3000K::make_frmloader(
    BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0,
    BitGen::bitstream::cktTileLibBstr *refCktTileLib = 0,
    FDU::cil_lib::majorLib *refMajorLib = 0) {
  return new FRMLoader3000K(refCktMemLib, refCktTileLib, refMajorLib);
}

FRMLoaderBase *BitgenFactory1000K::make_frmloader(
    BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0,
    BitGen::bitstream::cktTileLibBstr *refCktTileLib = 0,
    FDU::cil_lib::majorLib *refMajorLib = 0) {
  return new FRMLoader1000K(refCktMemLib, refCktTileLib, refMajorLib);
}

FRMLoaderBase *BitgenFactory80K::make_frmloader(
    BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0,
    BitGen::bitstream::cktTileLibBstr *refCktTileLib = 0,
    FDU::cil_lib::majorLib *refMajorLib = 0) {
  return new FRMLoader80K(refCktMemLib, refCktTileLib, refMajorLib);
}

FRMLoaderBase *BitgenFactory500K::make_frmloader(
    BitGen::bitstream::cktMemLibBstr *refCktMemLib = 0,
    BitGen::bitstream::cktTileLibBstr *refCktTileLib = 0,
    FDU::cil_lib::majorLib *refMajorLib = 0) {
  return new FRMLoader500K(refCktMemLib, refCktTileLib, refMajorLib);
}

} // namespace bitstream
} // namespace BitGen