#include "cil/clusterLib/cluster/cluster.h"

namespace FDU {
namespace cil_lib {
void Cluster::construct() {
  _links.setRefSiteLib(_refSiteLib);
  _links.setRefSiteName(_refSiteName);
}
} // namespace cil_lib
} // namespace FDU