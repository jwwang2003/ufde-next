#ifndef _TRANSINTILE_H_
#define _TRANSINTILE_H_

#include "cil/tileLib/tile/clstInTile.h"
#include "cil/transLib/transLib.h"

// #include <boost/foreach.hpp>

namespace FDU {
namespace cil_lib {

class trnsTile : public CilBase {
public:
  using sitesTrnsTileType = cilContainer<siteInTile>::range_type;
  using const_sitesTrnsTileType = cilContainer<siteInTile>::const_range_type;
  using siteTrnsTileIter = cilContainer<siteInTile>::iterator;
  using const_siteTrnsTileIter = cilContainer<siteInTile>::const_iterator;

private:
  cilContainer<siteInTile> _sites;
  sizeSpan _location;
  std::string _refSiteName;
  std::string _refTileName;
  trnsLib *_refTrnsLib;
  //		tileLib*	_refTileLib;

public:
  trnsTile(const std::string &name, sizeSpan location,
           const std::string &refTileName, trnsLib *refTrnsLib = 0)
      : CilBase(name), _location(location), _refTileName(refTileName),
        _refTrnsLib(refTrnsLib) {
    construct();
  }

  sitesTrnsTileType sites() { return _sites.range(); }
  const_sitesTrnsTileType sites() const { return _sites.range(); }

  siteInTile *addSite(siteInTile *site) { return _sites.add(site); }
  siteInTile &getSite(const std::string &siteName) {
    return *sites().find(siteName);
  }
  std::string getGRMName();

  std::string getRefSiteName() const { return _refSiteName; }
  std::string getRefTileName() const { return _refTileName; }

  void listDfotPips(vecBits &bits);

  Site *getRefSite(const std::string &siteName);
  siteInTile *getSiteWithoutException(const std::string &siteName);

  void construct();
};

class contTrnssTile {
public:
  using trnssTileType = cilContainer<trnsTile>::range_type;
  using const_trnssTileType = cilContainer<trnsTile>::const_range_type;
  using trnsTileIter = cilContainer<trnsTile>::iterator;
  using const_trnsTileIter = cilContainer<trnsTile>::const_iterator;

private:
  cilContainer<trnsTile> _trnss;
  std::string _refTileName;
  trnsLib *_refTrnsLib;
  //		tileLib*	_refTileLib;

public:
  contTrnssTile(const std::string &refTileName = "", trnsLib *refTrnsLib = 0)
      : _refTileName(refTileName), _refTrnsLib(refTrnsLib) {}

  void setRefTileName(const std::string &refTileName) {
    _refTileName = refTileName;
  }
  void setRefTrnsLib(trnsLib *ref) { _refTrnsLib = ref; }
  //		void setRefTileLib(tileLib* ref) { _refTileLib = ref; }

  trnssTileType trnss() { return _trnss.range(); }
  const_trnssTileType trnss() const { return _trnss.range(); }

  trnsTile *addTrns(trnsTile *trns) { return _trnss.add(trns); }
  trnsTile &getTrns(const std::string &trnsName) {
    return *trnss().find(trnsName);
  }
  std::string getGRMName();

  void listDfotPips(vecBits &bits);

  Site *getRefSite(const std::string &siteName);
  siteInTile *getSite(const std::string &siteName);
  sramInSiteSramTile *getRefSram(const bitTile &bit);
};

} // namespace cil_lib
} // namespace FDU

#endif