#ifndef _CKTMEMBSTR_H_
#define _CKTMEMBSTR_H_

#include "bitstream/memLibBstr/dfotMemLibBstr/dfotMemBstr/dfotMemBstr.h"

namespace BitGen {
namespace bitstream {

class cktMemBstr : public dfotMemBstr {
private:
  bool _used;
  dfotMemBstr *_refDfotMem;

public:
  cktMemBstr(dfotMemBstr *refDfotMem = 0) : _refDfotMem(refDfotMem) {
    construct();
  }

  bool isUsed() const { return _used; }
  void setUsed(bool used) { _used = used; }

  void addMemContents(const std::string &contents);
  virtual void getMemContents(BitList &bitList);
  virtual void construct();
};

inline void cktMemBstr::getMemContents(BitList &bitList) {
  if (_used)
    dfotMemBstr::getMemContents(bitList);
  else
    _refDfotMem->getMemContents(bitList);
}

} // namespace bitstream
} // namespace BitGen

#endif