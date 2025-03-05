#ifndef _DFOTMEMBSTR_H_
#define _DFOTMEMBSTR_H_

#include "bitstream/bstrBase.h"
#include "utils.h"

#include <algorithm>
#include <string>
#include <vector>

namespace BitGen {
namespace bitstream {

class dfotMemBstr : public bstrBase {
public:
  using BitList = std::vector<std::string>;

protected:
  BitList _bitList;
  int _listSize;
  //		bool _used;

public:
  dfotMemBstr() : bstrBase("") { construct(); }

  std::string &addBit(const std::string &bit) {
    _bitList.push_back(bit);
    return _bitList.back();
  }
  BitList &getBits() { return _bitList; }

  virtual void getMemContents(BitList &bitList);
  virtual void construct();
};

inline void dfotMemBstr::getMemContents(BitList &bitList) {
  for (const std::string &str : _bitList)
    bitList.push_back(str);
}

} // namespace bitstream
} // namespace BitGen

#endif