#include "cil/siteLib/site/site.h"

#include <boost/phoenix/core.hpp>
#include <boost/range/adaptors.hpp>

namespace FDU {
namespace cil_lib {
using namespace boost::adaptors;

void Site::construct() {
  _cfgInfo.setRefElemLib(_refElemLib);
  _cfgInfo.getSramNameSp().setRefElemLib(_refElemLib);
  _cfgInfo.getCfgElems().setRefElemLib(_refElemLib);
  _cfgInfo.getCfgElems().setRefNameSp(&_cfgInfo.getSramNameSp());
}

void Site::getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement) {
  _cfgInfo.getContentsOfFunc(bits, cfgElement);
}

pathInfo Site::getPath(const routeInfo &route) {
  pathInfo path;
  int pathAmount = 0;
  const Net *srcNet = _refSiteArch->nets().find(route._srcNet);
  const Net *snkNet = _refSiteArch->nets().find(route._snkNet);
  for (const Pin *src : srcNet->pins())
    if (!src->is_mpin()) {
      for (const Pin *snk : snkNet->pins())
        if (!snk->is_mpin()) {
          if (src->instance()->name() == snk->instance()->name()) {
            Element *elem =
                _refElemLib->getElement(src->instance()->down_module()->name());
            if (elem->hasRequiredPath(src->name(), snk->name())) {
              ++pathAmount;
              path._bCellName = src->instance()->name();
              path._in = src->name();
              path._out = snk->name();
            }
          }
        }
    }
  if (pathAmount == 0)
    // used for dummy path such like "F5COUPLING3 -> S0_X", so change exception
    // to warning
    FDU_LOG(WARN) << Warning("GRM: no required path here ... [Net] " +
                             srcNet->name() + "->" + snkNet->name());
  //			throw CilException("GRM: no required path here ... [Net]
  //" + srcNet->name() + "->" + snkNet->name());

  // in 80k, E2BEG4->TS0 like net does have more than one path, so change
  // exception to warning
  else {
    if (pathAmount > 1)
      FDU_LOG(WARN) << Warning("GRM: more than 1 path here ... [Net] " +
                               srcNet->name() + "->" + snkNet->name());
    // throw CilException("GRM: more than 1 path here ... [Net] " +
    // srcNet->name() + "->" + snkNet->name());
  }
  return path;
}

void Site::getPathPips(vecBits &bits, const routeInfo &route) {
  pathInfo path = getPath(route);
  if (!path.isEmpty()) { // used for dummy path such like "F5COUPLING3 -> S0_X",
                         // so add "if" branch
    string elemName = (_refSiteArch->instances().find(path._bCellName))
                          ->down_module()
                          ->name();
    Element *elem = _refElemLib->getElement(elemName);

    vecBits tBits;
    elem->listPathSrams(path, tBits);
    for (bitTile &bit : tBits) {
      bit._basicCell = path._bCellName;
      bit._siteName = route._siteName;
      bit._srcNet = route._srcNet;
      bit._snkNet = route._snkNet;
      bit._type = bitTile::route;
      bits.push_back(bit);
    }
  }
}

} // namespace cil_lib
} // namespace FDU