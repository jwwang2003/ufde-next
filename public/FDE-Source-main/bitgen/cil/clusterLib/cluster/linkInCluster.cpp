#include "cil/clusterLib/cluster/linkInCluster.h"

#include <boost/lexical_cast.hpp>

namespace FDU {
namespace cil_lib {

void linkCluster::construct() {
  _src._refSiteLib = _refSiteLib;
  _src._siteName = _refSiteName;

  _snk._refSiteLib = _refSiteLib;
  _snk._siteName = _refSiteName;
}

pinInLinkClus &linkCluster::setSrcOrSnk(pinType type, const std::string &pin,
                                        int row, int col) {

  if (type == source) {
    _src._pinName = pin;
    _src._row = row;
    _src._column = col;
    return _src;
  } else {
    _snk._pinName = pin;
    _snk._row = row;
    _snk._column = col;
    return _snk;
  }
}
} // namespace cil_lib
} // namespace FDU