#ifndef _SITE_H_
#define _SITE_H_

#include "arch/archlib.hpp"
#include "cil/cilBase.h"
#include "cil/siteLib/site/cfgInSite.h"

namespace FDU {
namespace cil_lib {
using namespace COS;
using namespace ARCH;

class Site : public CilBase {
private:
  cfgInfoSite _cfgInfo;
  elemLib *_refElemLib;
  ArchCell *_refSiteArch;

public:
  Site(const std::string &name, elemLib *refElemLib = 0)
      : CilBase(name), _refElemLib(refElemLib) {
    construct();
  }

  elemLib *getRefElemLib() const { return _refElemLib; }
  void setRefSiteArch(ArchCell *refSiteArch) { _refSiteArch = refSiteArch; }

  cfgInfoSite &getCfgInfo() { return _cfgInfo; }
  void listDfotCfgBits(vecBits &bits);
  void listDfotCfgs(vecCfgs &cfgs);
  void listDfotPips(vecBits &bits);
  void getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement);
  void getPathPips(vecBits &bits, const routeInfo &route);
  pathInfo getPath(const routeInfo &route);

  void construct();
};

inline void Site::listDfotCfgBits(vecBits &bits) {
  vecBits tBits;
  Element *ele = _refElemLib->getElement(_name);
  if (ele) {
    ele->listDfotSrams(tBits);
  }
  for (bitTile &bit : tBits) {
    bit._basicCell = _name;
    bit._type = bitTile::logic;
    bits.push_back(bit);
  }
}

inline void Site::listDfotCfgs(vecCfgs &cfgs) { _cfgInfo.listDfotCfgs(cfgs); }

inline void Site::listDfotPips(vecBits &bits) {
  vecBits tBits;
  for (const Instance *cell : _refSiteArch->instances()) {
    _refElemLib->getElement(cell->down_module()->name())->listDfotSrams(tBits);
    for (bitTile &bit : tBits) {
      bit._type = bitTile::route;
      bit._basicCell = cell->name();
      bits.push_back(bit);
    }
    tBits.clear();
  }
}

} // namespace cil_lib
} // namespace FDU

#endif