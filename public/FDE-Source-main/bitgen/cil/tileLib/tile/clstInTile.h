#ifndef _CLSTINTILE_H_
#define _CLSTINTILE_H_

#include "cil/clusterLib/clusterLib.h"
#include "cil/siteLib/siteLib.h"
#include <unordered_map>
#include <vector>

namespace FDU {
namespace cil_lib {
using std::string;

class sramInSiteSramTile : public CilBase {
private:
  string _basicCell;
  sizeSpan _localPlace;
  sizeSpan _tileOffset;
  string _ownerTile;
  string _landlordTile;
  string _refSiteName;
  siteLib *_refSiteLib;

public:
  sramInSiteSramTile(const string &name, const string &basicCell,
                     sizeSpan localPlace, sizeSpan tileOffset,
                     const string &ownerTile, const string &landlordTile,
                     const string &refSiteName, siteLib *refSiteLib = 0)
      : CilBase(name), _basicCell(basicCell), _localPlace(localPlace),
        _tileOffset(tileOffset), _ownerTile(ownerTile),
        _landlordTile(landlordTile), _refSiteName(refSiteName),
        _refSiteLib(refSiteLib) {}

  string getBasicCell() const { return _basicCell; }
  string getLandlordTile() const { return _landlordTile; }
  string getOwnerTile() const { return _ownerTile; }
  sizeSpan getLocalPlace() const { return _localPlace; }
  sizeSpan getTileOffset() const { return _tileOffset; }
};

class siteSramTile {
  // public:
  //	using sramsTileType		  =
  // cilContainer<sramInSiteSramTile>::range_type; 	using
  // const_sramsTileType = cilContainer<sramInSiteSramTile>::const_range_type;
  // using sramTileIter = cilContainer<sramInSiteSramTile>::iterator; 	using
  //const_sramTileIter  = cilContainer<sramInSiteSramTile>::const_iterator;

private:
  std::unordered_map<string, sramInSiteSramTile *> _srams;
  std::string _refSiteName;
  std::string _refTileName;
  siteLib *_refSiteLib;
  //		tileLib*	_refTileLib;

public:
  explicit siteSramTile(siteLib *refSiteLib = 0) : _refSiteLib(refSiteLib) {}

  // sramsTileType		srams()		  { return _srams.range(); }
  // const_sramsTileType srams() const { return _srams.range(); }

  sramInSiteSramTile *addSram(sramInSiteSramTile *sram) {
    string key = sram->getName() + sram->getBasicCell();
    _srams[key] = sram;
    return sram;
  }
  // sramInSiteSramTile* getSram(const std::string& sramName) { return
  // _srams[sramName]; }

  int getSize() const { return _srams.size(); }
  sramInSiteSramTile *getRefSram(const bitTile &bit);

  bool hasSram(const sramInSiteSramTile &sram);

  void setRefs(const std::string &refSiteName, const std::string &refTileName,
               siteLib *refSiteLib);
};

class siteInTile : public CilBase {
private:
  std::string _refSiteName;
  std::string _refTileName;
  sizeSpan _position;
  siteSramTile _siteSram;
  siteLib *_refSiteLib;
  //		tileLib*	  _refTileLib;

public:
  siteInTile(const std::string &name, const std::string &refSiteName,
             const std::string &refTileName, sizeSpan position,
             siteLib *refSiteLib = 0)
      : CilBase(name), _refSiteName(refSiteName), _refTileName(refTileName),
        _position(position), _refSiteLib(refSiteLib) {
    construct();
  }

  const std::string &getRefSiteName() { return _refSiteName; }
  siteSramTile &getSiteSram() { return _siteSram; }
  int getSiteSramSize() const { return _siteSram.getSize(); }

  void listDfotCfgBits(vecBits &bits);
  void listDfotCfgs(vecCfgs &cfgs);
  void listDfotPips(vecBits &bits);
  Site *getRefSite();
  sramInSiteSramTile *getRefSram(const bitTile &bit);

  bool hasSram(const sramInSiteSramTile &sram) {
    return _siteSram.hasSram(sram);
  }
  void construct();
};

class clstTile : public CilBase {
public:
  using sitesClstTileType = cilContainer<siteInTile>::range_type;
  using const_sitesClstTileType = cilContainer<siteInTile>::const_range_type;
  using siteClstTileIter = cilContainer<siteInTile>::iterator;
  using const_siteClstTileIter = cilContainer<siteInTile>::const_iterator;

private:
  cilContainer<siteInTile> _sites;
  sizeSpan _location;
  std::string _refSiteName;
  std::string _refTileName;
  clstLib *_refClstLib;
  //		tileLib*	_refTileLib;

public:
  clstTile(const std::string &name, sizeSpan location,
           const std::string &refTileName, clstLib *refClstLib = 0)
      : CilBase(name), _location(location), _refTileName(refTileName),
        _refClstLib(refClstLib) {
    construct();
  }

  sitesClstTileType sites() { return _sites.range(); }
  const_sitesClstTileType sites() const { return _sites.range(); }

  siteInTile *addSite(siteInTile *site) { return _sites.add(site); }
  siteInTile &getSite(const std::string &siteName) {
    return *sites().find(siteName);
  }

  std::string getRefSiteName() const { return _refSiteName; }
  std::string getRefTileName() const { return _refTileName; }

  void listDfotCfgBits(vecBits &bits);
  void listDfotCfgs(vecCfgs &cfgs);
  Site *getRefSite(const std::string &siteName);
  siteInTile *getSiteWithoutException(const std::string &siteName);

  void construct();
};

class contClstsTile {
public:
  using clstsTileType = cilContainer<clstTile>::range_type;
  using const_clstsTileType = cilContainer<clstTile>::const_range_type;
  using clstTileIter = cilContainer<clstTile>::iterator;
  using const_clstTileIter = cilContainer<clstTile>::const_iterator;

private:
  cilContainer<clstTile> _clusters;
  std::string _refTileName;
  clstLib *_refClstLib;
  //		tileLib*	_refTileLib;

public:
  contClstsTile(const std::string &refTileName = "", clstLib *refClstLib = 0)
      : _refTileName(refTileName), _refClstLib(refClstLib) {}

  void setRefTileName(const std::string &refTileName) {
    _refTileName = refTileName;
  }
  void setRefClstLib(clstLib *ref) { _refClstLib = ref; }
  //		void setRefTileLib(tileLib* ref) { _refTileLib = ref; }

  clstsTileType clusters() { return _clusters.range(); }
  const_clstsTileType clusters() const { return _clusters.range(); }

  clstTile *addClst(clstTile *clst) { return _clusters.add(clst); }
  clstTile &getClst(const std::string &clstName) {
    return *clusters().find(clstName);
  }

  void listDfotCfgBits(vecBits &bits);
  void listDfotCfgs(vecCfgs &cfgs);
  Site *getRefSite(const std::string &siteName);
  siteInTile *getSite(const std::string &siteName);
  sramInSiteSramTile *getRefSram(const bitTile &bit);
};

} // namespace cil_lib
} // namespace FDU

#endif