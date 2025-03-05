#include "cil/tileLib/tile/clstInTile.h"
#include "utils.h"

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>

namespace FDU {
namespace cil_lib {
using namespace boost;
using namespace boost::adaptors;

void siteSramTile::setRefs(const std::string &refSiteName,
                           const std::string &refTileName,
                           siteLib *refSiteLib) {
  _refSiteName = refSiteName;
  _refTileName = refTileName;
  _refSiteLib = refSiteLib;
}

void siteInTile::construct() {
  _siteSram.setRefs(_refSiteName, _refTileName, _refSiteLib);
}

void clstTile::construct() {
  _refSiteName = _refClstLib->getCluster(_name).getRefSiteName();
}

void siteInTile::listDfotCfgBits(vecBits &bits) {
  vecBits tBits;
  _refSiteLib->getSite(_refSiteName).listDfotCfgBits(tBits);
  for (bitTile &bit : tBits) {
    bit._siteName = _name;
    bits.push_back(bit);
  }
}
void siteInTile::listDfotCfgs(vecCfgs &cfgs) {
  // ASSERTD(_siteSram.getSize() > 0, BstrException("siteSram in cluster of
  // tile: no sram(Config Sram) in this site"));
  vecCfgs tCfgs;
  _refSiteLib->getSite(_refSiteName).listDfotCfgs(tCfgs);
  for (cfgElem &cfg : tCfgs) {
    cfg._siteName = _name;
    cfgs.push_back(cfg);
  }
}

void clstTile::listDfotCfgBits(vecBits &bits) {
  for (siteInTile *site : sites())
    if (site->getSiteSramSize() > 0)
      site->listDfotCfgBits(bits);
}

void clstTile::listDfotCfgs(vecCfgs &cfgs) {
  for (siteInTile *site : sites())
    if (site->getSiteSramSize() > 0)
      site->listDfotCfgs(cfgs);
}

void contClstsTile::listDfotCfgBits(vecBits &bits) {
  for (clstTile *clst : clusters())
    clst->listDfotCfgBits(bits);
}

void contClstsTile::listDfotCfgs(vecCfgs &cfgs) {
  for (clstTile *clst : clusters())
    clst->listDfotCfgs(cfgs);
}

void siteInTile::listDfotPips(vecBits &bits) {
  // ASSERTD(_siteSram.getSize() > 0, BstrException("siteSram in cluster of
  // tile: no sram(PIP Sram) in this site"));
  vecBits tBits;
  _refSiteLib->getSite(_refSiteName).listDfotPips(tBits);
  for (bitTile &bit : tBits) {
    bit._siteName = _name;
    bits.push_back(bit);
  }
}

Site *siteInTile::getRefSite() { return &_refSiteLib->getSite(_refSiteName); }

Site *clstTile::getRefSite(const std::string &siteName) {
  siteClstTileIter it = find_if(sites(), [&siteName](const siteInTile *site) {
    return site->getName() == siteName;
  });
  return it != _sites.end() ? it->getRefSite() : nullptr;
}

Site *contClstsTile::getRefSite(const std::string &siteName) {
  Site *site = nullptr;
  for (clstTile *cluster : clusters()) {
    site = cluster->getRefSite(siteName);
    if (site)
      break;
  }
  return site;
}

sramInSiteSramTile *siteSramTile::getRefSram(const bitTile &bit) {
  string key = bit._sramName + bit._basicCell;
  if (bit._basicCell == _srams[key]->getBasicCell())
    return _srams[key];
  else
    return nullptr;
}

sramInSiteSramTile *siteInTile::getRefSram(const bitTile &bit) {
  return _siteSram.getRefSram(bit);
}

bool siteSramTile::hasSram(const sramInSiteSramTile &sram) {
  string key = sram.getName() + sram.getBasicCell();
  return sram.getBasicCell() == _srams[key]->getBasicCell();
}

siteInTile *clstTile::getSiteWithoutException(const std::string &siteName) {
  siteClstTileIter it = find_if(sites(), [&siteName](const siteInTile *site) {
    return site->getName() == siteName;
  });
  return it != _sites.end() ? *it : nullptr;
}

siteInTile *contClstsTile::getSite(const std::string &siteName) {
  siteInTile *site = nullptr;
  for (clstTile *cluster : clusters()) {
    site = cluster->getSiteWithoutException(siteName);
    if (site)
      break;
  }
  return site;
}

sramInSiteSramTile *contClstsTile::getRefSram(const bitTile &bit) {
  siteInTile *site = getSite(bit._siteName);
  return site ? site->getRefSram(bit) : nullptr;
}

} // namespace cil_lib
} // namespace FDU