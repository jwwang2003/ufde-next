#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include "cil/clusterLib/cluster/dimensionInCluster.h"
#include "cil/clusterLib/cluster/linkInCluster.h"

namespace FDU {
namespace cil_lib {

class Cluster : public CilBase {
private:
  std::string _refSiteName;
  dimenCluster _dimension;
  contLinksCluster _links;
  siteLib *_refSiteLib;

public:
  Cluster(const std::string &clsName, const std::string &refSiteName,
          siteLib *refSiteLib = 0)
      : CilBase(clsName), _refSiteName(refSiteName), _refSiteLib(refSiteLib) {}

  dimenCluster &getDimension() { return _dimension; }
  contLinksCluster &getLinks() { return _links; }

  const std::string &getRefSiteName() const { return _refSiteName; }

  void construct();
};

} // namespace cil_lib
} // namespace FDU

#endif