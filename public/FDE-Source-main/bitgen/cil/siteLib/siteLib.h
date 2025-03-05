#ifndef _SITELIB_H_
#define _SITELIB_H_

#include "cil/siteLib/site/site.h"

namespace FDU {
namespace cil_lib {

class siteLib {
public:
  using sitesType = cilContainer<Site>::range_type;
  using const_sitesType = cilContainer<Site>::const_range_type;
  using siteIter = cilContainer<Site>::iterator;
  using const_siteIter = cilContainer<Site>::const_iterator;

private:
  cilContainer<Site> _sites;
  elemLib *_refElemLib;

public:
  explicit siteLib(elemLib *refElemLib = 0) : _refElemLib(refElemLib) {}

  sitesType sites() { return _sites.range(); }
  const_sitesType sites() const { return _sites.range(); }

  Site *addSite(Site *site) { return _sites.add(site); }
  Site &getSite(const std::string &siteName) { return *sites().find(siteName); }
};

} // namespace cil_lib
} // namespace FDU

#endif