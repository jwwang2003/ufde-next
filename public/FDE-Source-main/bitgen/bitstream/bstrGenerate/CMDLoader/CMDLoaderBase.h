#ifndef _CMDLOADERBASE_H_
#define _CMDLOADERBASE_H_

#include "bitstream/bstrGenerate/loaderBase.h"

#include "PropertyName.h"
#include "main/arguments/Args.h"
#include <iomanip>
#include <sstream>
#include <vector>

namespace BitGen {
namespace bitstream {
using namespace FDU;

class CMDLoaderBase : public loaderBase {
public:
  /*		static const int _BITS_PER_FRM = 396; // not used, just for
   * FDP1000K*/
  static const int _BITS_PER_WORD = 32;
  static const int _BITS_PER_GRP_REVERSED_3000K = 36;
  static const int _BITS_PER_GRP_REVERSED_80K = 16;
  static const int _DEC_OF_0x10000 = 65536;

protected:
  std::ostringstream _ofs;

protected:
  unsigned int toWord(const std::vector<int> &bits, int wordOffs);
  int getFRMWords(std::vector<unsigned int> &FRMWords, int tileCol, int FRM,
                  int bits_per_grp_reversed, int initialNum);
  int getMajorWords(std::vector<unsigned int> &majorWords, int tileCol,
                    int FRMAmnt, int bits_per_grp_reversed, int initialNum);
  void wrdToOstream(unsigned int base, unsigned int wordNum);
  void wrdsToOstream(const std::vector<unsigned int> &words);
  void memToOstream(const dfotMemBstr::BitList &bitList);
  void writeNomalTiles(int major_shift, int bits_per_grp_reversed,
                       int initialNum);
  void exportBitFile(const std::string &outputFile);

  void bsHeader();
  void fillBlank(int amount);
  void adjustSYNC(std::string syncWord);
  void insertCMD(std::string command, std::string comment);
  void setFRMLen(int FRMLength);
  void setOption(std::string option);
  void shapeMask(std::string mask);
  void mountBMFA(int majorAddr);
  void mapOutFDRI(int wordLength);
  void appointCTL(std::string control);
  void dummy();
  void mountSMFA(int majorAddr, int major_shift);
  void reverse(std::vector<int> &FRMBits, int bits_per_grp_reversed);
  void writeMem(int memAmount, int wrdsAmnt_shift, int fillblank);
  int adjust(std::vector<int> &FRMBits, int initialNum);

  CMDLoaderBase(cktMemLibBstr *refCktMemLib = 0,
                cktTileLibBstr *refCktTileLib = 0,
                FDU::cil_lib::majorLib *refMajorLib = 0,
                FDU::cil_lib::bstrCMDLib *refbstrCMDLib = 0)
      : loaderBase(refCktMemLib, refCktTileLib, refMajorLib, refbstrCMDLib) {}
};

//////////////////////////////////////////////////////////////////////////
// inline functions

inline int CMDLoaderBase::adjust(std::vector<int> &FRMBits, int initialNum) {
  int bitsAmnt = FRMBits.size(), wordsAmnt = 0;
  if (bitsAmnt % _BITS_PER_WORD == 0) {
    wordsAmnt = bitsAmnt / _BITS_PER_WORD;
  } else {
    wordsAmnt = bitsAmnt / _BITS_PER_WORD + 1;
    FRMBits.resize(wordsAmnt * _BITS_PER_WORD, initialNum);
  }
  return wordsAmnt;
}

inline unsigned int CMDLoaderBase::toWord(const std::vector<int> &bits,
                                          int wordOffs) {
  unsigned int word = 0;
  int begin = wordOffs * _BITS_PER_WORD;
  for (int bitIdx = 0; bitIdx < _BITS_PER_WORD; ++bitIdx)
    word = (word << 1) + bits.at(begin + bitIdx);
  return word;
}

inline void CMDLoaderBase::wrdToOstream(unsigned int base,
                                        unsigned int wordNum) {
  unsigned int word = base + wordNum;
  unsigned int lowHalfWord = word % _DEC_OF_0x10000;
  unsigned int highHalfWord = word / _DEC_OF_0x10000;
  _ofs << std::setw(4) << std::setfill('0') << std::hex << highHalfWord << "_";
  _ofs << std::setw(4) << std::setfill('0') << std::hex << lowHalfWord << "\n";
}

inline void
CMDLoaderBase::wrdsToOstream(const std::vector<unsigned int> &words) {
  for (unsigned int word : words)
    wrdToOstream(0, word);
}

inline void CMDLoaderBase::memToOstream(
    const BitGen::bitstream::dfotMemBstr::BitList &bitList) {
  for (const std::string &str : bitList)
    _ofs << str << "\n";
}

inline void CMDLoaderBase::mountSMFA(int majorAddr,
                                     int major_shift) // 80k,shift:19   1000k:17
{
  _ofs << "3000_2001"
       << "\n";
  wrdToOstream(0, majorAddr << major_shift);
}

inline void CMDLoaderBase::reverse(std::vector<int> &FRMBits,
                                   int bits_per_grp_reversed) {
  for (size_t i = 0, size = FRMBits.size(); i < size; i += bits_per_grp_reversed)
    for (int j = i, end = j + bits_per_grp_reversed - 1; j < end; ++j, --end)
      std::swap(FRMBits[j], FRMBits[end]);
}

} // namespace bitstream
} // namespace BitGen

#endif