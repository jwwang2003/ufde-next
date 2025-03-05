#ifndef _LINKINCLUSTER_H_
#define _LINKINCLUSTER_H_

#include <string>

#include "cil/siteLib/siteLib.h"
#include "container/Container.h"
#include "utils/sizeSpan.h"

namespace FDU {
namespace cil_lib {

struct pinInLinkClus {
  std::string _pinName;
  std::string _siteName;
  int _row;
  int _column;
  siteLib *_refSiteLib;
};

class linkCluster : public CilBase {
public:
  enum pinType { source, sink };

private:
  std::string _refSiteName;
  sizeSpan _containerSize;
  pinInLinkClus _src;
  pinInLinkClus _snk;
  siteLib *_refSiteLib;

public:
  linkCluster(const std::string &refSiteName, siteLib *refSiteLib,
              sizeSpan containerSize)
      : CilBase("link"), _refSiteName(refSiteName), _containerSize(containerSize),
        _refSiteLib(refSiteLib) {
    construct();
  }

  void construct();
  pinInLinkClus &setSrcOrSnk(pinType type, const std::string &pin, int row,
                             int col);
};

class contLinksCluster {
public:
  using linksClstType = cilContainer<linkCluster>::range_type;
  using const_linksClstType = cilContainer<linkCluster>::const_range_type;
  using linkClstIter = cilContainer<linkCluster>::iterator;
  using const_linkClstIter = cilContainer<linkCluster>::const_iterator;

private:
  cilContainer<linkCluster> _links;
  std::string _refSiteName;
  siteLib *_refSiteLib;

public:
  explicit contLinksCluster(siteLib *refSiteLib = 0)
      : _refSiteLib(refSiteLib) {}

  linksClstType links() { return _links.range(); }
  const_linksClstType links() const { return _links.range(); }

  linkCluster *addLink(linkCluster *link) { return _links.add(link); }

  void setRefSiteName(const std::string &siteName) { _refSiteName = siteName; }
  void setRefSiteLib(siteLib *ref) { _refSiteLib = ref; }
};

} // namespace cil_lib
} // namespace FDU

#endif