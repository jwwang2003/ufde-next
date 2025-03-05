#ifndef _CKTTILEBSTR_H_
#define _CKTTILEBSTR_H_

#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/dfotTileBstr.h"

namespace BitGen {
namespace bitstream {

class cktTileBstr : public dfotTileBstr {
private:
  bool _used;
  dfotTileBstr *_refDfotTile;

public:
  // must provide refDfotTile when construct!
  explicit cktTileBstr(dfotTileBstr *refDfotTile = 0)
      : _refDfotTile(refDfotTile) {
    construct();
  }

  bool isUsed() const { return _used; }
  void setUsed(bool used) { _used = used; }

  virtual int getFRMBits(std::vector<int> &FRMBits, int FRM);

  virtual void analyzeBits();
  virtual void regulateBits(vecBits &lodgerBits);
  virtual void buildBitArry();

  virtual void construct();
};

inline int cktTileBstr::getFRMBits(std::vector<int> &FRMBits, int FRM) {
  if (_used)
    return dfotTileBstr::getFRMBits(FRMBits, FRM);
  else
    return _refDfotTile->getFRMBits(FRMBits, FRM);
}

} // namespace bitstream
} // namespace BitGen

#endif