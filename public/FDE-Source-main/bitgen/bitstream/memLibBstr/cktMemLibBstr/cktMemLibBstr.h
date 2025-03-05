#ifndef _CKTMEMLIBBSTR_H_
#define _CKTMEMLIBBSTR_H_

#include "bitstream/memLibBstr/cktMemLibBstr/cktMemBstr/cktMemBstr.h"
#include "bitstream/memLibBstr/dfotMemLibBstr/dfotMemLibBstr.h"
#include "log.h"
#include "utils.h"

namespace BitGen {
namespace bitstream {

class cktMemLibBstr {
public:
  using cktMemsType = bstrContainer<cktMemBstr>::range_type;
  using const_cktMemsType = bstrContainer<cktMemBstr>::const_range_type;
  using cktMemIter = bstrContainer<cktMemBstr>::iterator;
  using const_cktMemIter = bstrContainer<cktMemBstr>::const_iterator;

  // for FDP1000K
  static const int _1000K_MEM_AMOUNT = 10;
  static const char *_1000K_MEMS[];
  // for FDP3000K
  static const int _3000K_MEM_AMOUNT = 16;
  static const char *_3000K_PQ208_MEMS[];
  static const char *_3000K_CB228_MEMS[];
  // for FDP500K
  static const int _500KIP_MEM_AMOUNT = 32;
  static const char *_500KIP_FG256_MEMS[];

private:
  bstrContainer<cktMemBstr> _cktMems;
  dfotMemLibBstr *_refDfotMemLib;

public:
  cktMemLibBstr(dfotMemLibBstr *refDfotMemLib = 0)
      : _refDfotMemLib(refDfotMemLib) {}

  cktMemsType cktMems() { return _cktMems.range(); }
  const_cktMemsType cktMems() const { return _cktMems.range(); }

  cktMemBstr *addCktMem(cktMemBstr *cktMem) { return _cktMems.add(cktMem); }
  cktMemBstr &getCktMem(const std::string &memName);

  void addMemContents(const string &cktTileName, const string &contents);
  void getMemContents(dfotMemBstr::BitList &bitList, int memIdx);

  void construct(const string &device, const string &package);
};

using namespace boost;

inline cktMemBstr &cktMemLibBstr::getCktMem(const std::string &memName) {
  cktMemIter it = find_if(cktMems(), [&memName](const cktMemBstr *cktMem) {
    return cktMem->getName() == memName;
  });
  ASSERTD(it != _cktMems.end(),
          "cktMemLib: not exist such BRAM named ... " + memName);
  return **it;
}

inline void cktMemLibBstr::getMemContents(dfotMemBstr::BitList &bitList,
                                          int memIdx) {
  _cktMems.at(memIdx)->getMemContents(bitList);
}

} // namespace bitstream
} // namespace BitGen

#endif