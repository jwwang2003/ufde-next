#include "cil/clusterLib/cluster/dimensionInCluster.h"

#include <boost/lexical_cast.hpp>
#include <string>

namespace FDU {
namespace cil_lib {
using std::string;

void dimenCluster::setSizeByDirect(direction direct, int cls_size,
                                   int nom_size) {
  switch (direct) {
  case dimenCluster::horizontal:
    _clusterSizeSpan._columnSpan = cls_size;
    _normSizeSpan._columnSpan = nom_size;
    break;
  case dimenCluster::vertical:
    _clusterSizeSpan._rowSpan = cls_size;
    _normSizeSpan._rowSpan = nom_size;
    break;
  }
}

} // namespace cil_lib
} // namespace FDU