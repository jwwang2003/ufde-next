#ifndef _TRANSLIB_H_
#define _TRANSLIB_H_

#include "cil/transLib/transmission/transmission.h"
#include "container/Container.h"

namespace FDU {
namespace cil_lib {

class trnsLib {
public:
  using trnssType = cilContainer<Trans>::range_type;
  using const_trnssType = cilContainer<Trans>::const_range_type;
  using trnsIter = cilContainer<Trans>::iterator;
  using const_trnsIter = cilContainer<Trans>::const_iterator;

private:
  cilContainer<Trans> _trans;
  siteLib *_refSiteLib;

public:
  explicit trnsLib(siteLib *refSiteLib = 0) : _refSiteLib(refSiteLib) {}

  trnssType trnss() { return _trans.range(); }
  const_trnssType trnss() const { return _trans.range(); }

  Trans *addTrans(Trans *trans) { return _trans.add(trans); }
  Trans &getTrans(const std::string &transName) {
    return *trnss().find(transName);
  }
};

} // namespace cil_lib
} // namespace FDU

#endif