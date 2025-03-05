#include "cil/tileLib/tile/transInTile.h"
#include "log.h"

#include <boost/range/adaptors.hpp>

namespace FDU {
namespace cil_lib {
using namespace boost::adaptors;

void trnsTile::construct() {
  _refSiteName = _refTrnsLib->getTrans(_name).getRefSiteName();
}

std::string trnsTile::getGRMName() {
  ASSERTD(_sites.size() <= 1,
          "transmission in tile: there are more than 1 grm site here");
  return _sites.size() == 1 ? _sites.at(0)->getName() : "";
}

std::string contTrnssTile::getGRMName() {
  ASSERTD(_trnss.size() <= 1, "tile: there are more than 1 transmission here");
  return _trnss.size() == 1 ? _trnss.at(0)->getGRMName() : "";
}

void trnsTile::listDfotPips(vecBits &bits) {
  for (siteInTile *site : sites())
    if (site->getSiteSramSize() > 0)
      site->listDfotPips(bits);
}

void contTrnssTile::listDfotPips(vecBits &bits) {
  for (trnsTile *trns : trnss())
    trns->listDfotPips(bits);
}

Site *trnsTile::getRefSite(const std::string &siteName) {
  siteTrnsTileIter it = find_if(sites(), [&siteName](const siteInTile *site) {
    return site->getName() == siteName;
  });
  return it != _sites.end() ? it->getRefSite() : nullptr;
}

Site *contTrnssTile::getRefSite(const std::string &siteName) {
  Site *site;
  for (trnsTile *trans : trnss()) {
    site = trans->getRefSite(siteName);
    if (site)
      break;
  }
  return site;
}

siteInTile *trnsTile::getSiteWithoutException(const std::string &siteName) {
  siteTrnsTileIter it = find_if(sites(), [&siteName](const siteInTile *site) {
    return site->getName() == siteName;
  });
  return it != _sites.end() ? *it : nullptr;
}

siteInTile *contTrnssTile::getSite(const std::string &siteName) {
  siteInTile *site = nullptr;
  for (trnsTile *trans : trnss()) {
    site = trans->getSiteWithoutException(siteName);
    if (site)
      break;
  }
  return site;
}

sramInSiteSramTile *contTrnssTile::getRefSram(const bitTile &bit) {
  siteInTile *site = getSite(bit._siteName);
  return site ? site->getRefSram(bit) : nullptr;
}

} // namespace cil_lib
} // namespace FDU
