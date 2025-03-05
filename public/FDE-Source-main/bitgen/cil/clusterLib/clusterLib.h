#ifndef _CLUSTERLIB_H_
#define _CLUSTERLIB_H_

#include "cil/clusterLib/cluster/cluster.h"

namespace FDU {
namespace cil_lib {

class clstLib {
public:
  using clustersType = cilContainer<Cluster>::range_type;
  using const_clustersType = cilContainer<Cluster>::const_range_type;
  using clusterIter = cilContainer<Cluster>::iterator;
  using const_clusterIter = cilContainer<Cluster>::const_iterator;

private:
  cilContainer<Cluster> _clusters;
  siteLib *_refSiteLib;

public:
  explicit clstLib(siteLib *ref = 0) : _refSiteLib(ref) {}

  clustersType clusters() { return _clusters.range(); }
  const_clustersType clusters() const { return _clusters.range(); }

  Cluster *addCluster(Cluster *cluster) { return _clusters.add(cluster); }
  Cluster &getCluster(const std::string &clsName) {
    return *clusters().find(clsName);
  }
};

} // namespace cil_lib
} // namespace FDU

#endif