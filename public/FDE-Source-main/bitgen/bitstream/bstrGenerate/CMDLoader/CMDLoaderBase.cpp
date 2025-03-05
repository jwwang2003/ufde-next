#include "bitstream/bstrGenerate/CMDLoader/CMDLoaderBase.h"
#include "PropertyName.h"
#include "bitstream/bstrGenerate/bstrGener.h"
#include "log.h"
#include "main/arguments/Args.h"
#include "version.h"
#include "zfstream.h"

namespace BitGen {
namespace bitstream {
using namespace FDU;

int CMDLoaderBase::getFRMWords(std::vector<unsigned int> &FRMWords, int tileCol,
                               int FRM, int bits_per_grp_reversed,
                               int initialNum) {
  std::vector<int> FRMBits;
  _refCktTileLib->getFRMBits(FRMBits, tileCol, FRM);
  reverse(FRMBits, bits_per_grp_reversed);
  int FRMWrdsAmnt = adjust(FRMBits, initialNum);
  for (int wordOffs = 0; wordOffs < FRMWrdsAmnt; ++wordOffs)
    FRMWords.push_back(toWord(FRMBits, wordOffs));
  return FRMWrdsAmnt;
}

int CMDLoaderBase::getMajorWords(std::vector<unsigned int> &majorWords,
                                 int tileCol, int FRMAmnt,
                                 int bits_per_grp_reversed, int initialNum) {
  int majorWrdsAmnt = 0;
  std::vector<unsigned int> FRMWords;
  for (int FRMIdx = 0; FRMIdx < FRMAmnt; ++FRMIdx) {
    majorWrdsAmnt += getFRMWords(FRMWords, tileCol, FRMIdx,
                                 bits_per_grp_reversed, initialNum);
    majorWords.insert(majorWords.end(), FRMWords.begin(), FRMWords.end());
    FRMWords.clear();
  }
  return majorWrdsAmnt;
}

void CMDLoaderBase::bsHeader() {
  _ofs << "0000_0000"
       << "\t// chip_type: " << args._device << '\n';
  _ofs << "0000_0000"
       << "\t// bit file name: " << args._bitstream << '\n';
  _ofs << "0000_0000"
       << "\t// current time: " << timestamp() << '\n';
  fillBlank(3);
}

void CMDLoaderBase::fillBlank(int amount) {
  for (int i = 0; i < amount; ++i)
    _ofs << "0000_0000"
         << "\n";
}

void CMDLoaderBase::adjustSYNC(std::string syncWord) {
  _ofs << syncWord << "\n" << syncWord << "\n";
}

void CMDLoaderBase::insertCMD(std::string command, std::string comment) {
  _ofs << "3000_8001"
       << "\n"
       << command << "\t//" << comment << "\n";
}

void CMDLoaderBase::setFRMLen(int FRMLength) {
  _ofs << "3001_6001"
       << "\t//frame length"
       << "\n";
  wrdToOstream(0, FRMLength);
}

void CMDLoaderBase::setOption(std::string option) {
  _ofs << "3001_2001"
       << "\n"
       << option << "\t//configure option"
       << "\n";
}

void CMDLoaderBase::shapeMask(std::string mask) {
  _ofs << "3000_C001"
       << "\n"
       << mask << "\t//mask"
       << "\n";
}

void CMDLoaderBase::mountBMFA(int majorAddr) {
  _ofs << "3000_2001"
       << "\n";
  wrdToOstream(0x2000000, majorAddr * 0x20000);
}

void CMDLoaderBase::mapOutFDRI(int wordLength) {
  _ofs << "3000_4000"
       << "\t//" << std::dec << wordLength << " words"
       << "\n";
  wrdToOstream(0x50000000, wordLength);
}

void CMDLoaderBase::appointCTL(std::string control) {
  _ofs << "3000_A001"
       << "\n"
       << control << "\t//CTL value"
       << "\n";
}

void CMDLoaderBase::dummy() {
  _ofs << "3000_2001"
       << "\n"
       << "0000_0000"
       << "\n";
}

void CMDLoaderBase::writeNomalTiles(int major_shift, int bits_per_grp_reversed,
                                    int initialNum) {
  std::vector<unsigned int> majorWords;
  for (FDU::cil_lib::Major *major : _refMajorLib->majors()) {
    int majorAddr = major->getMajorAddr();
    int FRMAmount = major->getFRMAmount();
    int tileCol = major->getTileCol();
    // partial bitstream
    if (args._partialbitstream >= 0) {
      if (tileCol == args._partialbitstream) {
        int majorWrdsAmnt = getMajorWords(majorWords, tileCol, FRMAmount,
                                          bits_per_grp_reversed, initialNum);
        mountSMFA(majorAddr, major_shift);
        mapOutFDRI(majorWrdsAmnt);
        wrdsToOstream(majorWords);
      }
      majorWords.clear();
    } else {
      int majorWrdsAmnt = getMajorWords(majorWords, tileCol, FRMAmount,
                                        bits_per_grp_reversed, initialNum);
      mountSMFA(majorAddr, major_shift);
      mapOutFDRI(majorWrdsAmnt);
      wrdsToOstream(majorWords);
      majorWords.clear();
    }
  }
}

void CMDLoaderBase::writeMem(int memAmount, int wrdsAmnt_shift, int fillblank) {
  dfotMemBstr::BitList memBitList;
  for (int memIdx = 0; memIdx < memAmount; ++memIdx) {
    _refCktMemLib->getMemContents(memBitList, memIdx);
    int wrdsAmnt = memBitList.size();
    mountBMFA(memIdx + 1);
    mapOutFDRI(wrdsAmnt + wrdsAmnt_shift);
    memToOstream(memBitList);
    fillBlank(fillblank);
    memBitList.clear();
  }
}

void CMDLoaderBase::exportBitFile(const std::string &outputFile) {
  if (args._encrypt) {
    zofstream zofs(outputFile.c_str());
    ASSERT(zofs, "CMDLoader: can not open ... " + outputFile);
    zofs << _ofs.str();
  } else {
    std::ofstream ofs(outputFile.c_str());
    ASSERT(ofs.is_open(), "CMDLoader: can not open ... " + outputFile);
    ofs << _ofs.str();
  }
}

} // namespace bitstream
} // namespace BitGen