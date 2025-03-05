#ifndef _MAJORLIB_H_
#define _MAJORLIB_H_

#include "container/Container.h"

namespace FDU {
namespace cil_lib {

class Major : public CilBase {
private:
  int _majorAddr;
  int _FRMAmount;
  int _tileCol;

public:
  explicit Major(int majorAddr, int FRMAmount, int tileCol)
      : CilBase(""), _majorAddr(majorAddr), _FRMAmount(FRMAmount),
        _tileCol(tileCol) {}

  int getMajorAddr() const { return _majorAddr; }
  int getFRMAmount() const { return _FRMAmount; }
  int getTileCol() const { return _tileCol; }
};

class majorLib {
public:
  using majorsType = cilContainer<Major>::range_type;
  using const_majorsType = cilContainer<Major>::const_range_type;
  using majorIter = cilContainer<Major>::iterator;
  using const_majorIter = cilContainer<Major>::const_iterator;

private:
  cilContainer<Major> _majors;

public:
  majorsType majors() { return _majors.range(); }
  const_majorsType majors() const { return _majors.range(); }

  Major *addMajor(Major *major) { return _majors.add(major); }
};

} // namespace cil_lib
} // namespace FDU

#endif