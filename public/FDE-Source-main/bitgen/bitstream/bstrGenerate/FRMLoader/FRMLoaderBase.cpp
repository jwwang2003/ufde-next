#include "bitstream/bstrGenerate/FRMLoader/FRMLoaderBase.h"
#include "log.h"

// #include <boost/foreach.hpp>

namespace BitGen {
namespace bitstream {

//////////////////////////////////////////////////////////////////////////
// static data members

const string FRMLoaderBase::_MAJOR_ADDR = "\tmajor = ";
const string FRMLoaderBase::_FRAME_ADDR = "\tframe = ";

//////////////////////////////////////////////////////////////////////////
// member functions
void FRMLoaderBase::bstrGen(const std::string &outputFile) {
  _ofs.open(outputFile.c_str());
  ASSERT(_ofs.is_open(), "FRMLoader: can not open ... " + outputFile);

  std::vector<int> colBits;
  for (FDU::cil_lib::Major *major : _refMajorLib->majors()) {
    int majorAddr = major->getMajorAddr();
    int tileCol = major->getTileCol();
    sizeSpan bitSize = _refCktTileLib->getColBits(colBits, tileCol);
    bitsToOstream(colBits, bitSize, majorAddr);
    colBits.clear();
  }
}

} // namespace bitstream
} // namespace BitGen