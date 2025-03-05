#ifndef _BRAMLIB_H_
#define _BRAMLIB_H_

#include "container/Container.h"

namespace FDU {
namespace cil_lib {

class Bram : public CilBase {
private:
  int _bramAddr;
  int _bl;
  std::string _type;

public:
  explicit Bram(int bramAddr, int bl, const std::string &type)
      : CilBase(""), _bramAddr(bramAddr), _bl(bl), _type(type) {}

  int getBramAddr() const { return _bramAddr; }
  int getBl() const { return _bl; }
  std::string getType() const { return _type; }
};

class bramLib {
public:
  using bramsType = cilContainer<Bram>::range_type;
  using const_bramsType = cilContainer<Bram>::const_range_type;
  using bramIter = cilContainer<Bram>::iterator;
  using const_bramIter = cilContainer<Bram>::const_iterator;

private:
  cilContainer<Bram> _brams;

public:
  bramsType brams() { return _brams.range(); }
  const_bramsType brams() const { return _brams.range(); }

  Bram *addBram(Bram *bram) { return _brams.add(bram); }
};

} // namespace cil_lib
} // namespace FDU

#endif