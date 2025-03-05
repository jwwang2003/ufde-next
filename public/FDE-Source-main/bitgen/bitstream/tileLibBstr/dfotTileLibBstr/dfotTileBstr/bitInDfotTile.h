#ifndef _BITINDFOTTILE_H_
#define _BITINDFOTTILE_H_

#include "utils/cfgInTile.h"

#include <string>
#include <vector>
#include <algorithm>

namespace BitGen {
namespace bitstream {

struct contBitsDfotTileBstr {
  using Bits = std::vector<bitTile>;
  Bits _bits;

  bitTile &addBit(const bitTile &bit) {
    _bits.push_back(bit);
    return _bits.back();
  }
  void appendBits(const vecBits &bits);
  void resetBits(const vecBits &bits);

  bool existLodgerBits();
  bool hasOffsetBits();
  bool isEmpty();
};

inline bool contBitsDfotTileBstr::existLodgerBits() {
  Bits::iterator it =
      std::find_if(_bits.begin(), _bits.end(),
                   [](const bitTile &bit) { return bit.isLodger(); });
  return it != _bits.end();
}

inline bool contBitsDfotTileBstr::hasOffsetBits() {
  Bits::iterator it =
      std::find_if(_bits.begin(), _bits.end(),
                   [](const bitTile &bit) { return bit.hasOffset(); });
  return it != _bits.end();
}

inline bool contBitsDfotTileBstr::isEmpty() { return _bits.empty(); }

inline void contBitsDfotTileBstr::appendBits(const vecBits &bits) {
  _bits.insert(_bits.end(), bits.begin(), bits.end());
}

inline void contBitsDfotTileBstr::resetBits(const vecBits &bits) {
  _bits = bits;
}

} // namespace bitstream
} // namespace BitGen

#endif