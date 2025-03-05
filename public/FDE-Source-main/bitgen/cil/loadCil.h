#ifndef _LOADCIL_H_
#define _LOADCIL_H_

#include "cilLib.h"
#include "xmlutils.h"

namespace FDU {
namespace cil_lib {
using namespace FDU::XML;

class CilLibHandler {
  cilLibrary *_cilLib;

public:
  CilLibHandler(cilLibrary *cil = 0) : _cilLib(cil) {}
  cilLibrary *loadCil(xml_node *node);

private:
  void loadEleLib(xml_node *node);
  void loadSiteLib(xml_node *node);
  void loadClstLib(xml_node *node);
  void loadTrnsLib(xml_node *node);
  void loadTileLib(xml_node *node);
  void loadMajorLib(xml_node *node);
  void loadBramLib(xml_node *node);
  void loadbstrCMDLib(xml_node *node);

  // element Lib
  void loadElem(xml_node *node, elemLib *lib);
  void loadSrams(xml_node *node, Element *ele);
  void loadPaths(xml_node *node, Element *ele);
  void loadSram(xml_node *node, contSramsElem *srams);
  void loadPath(xml_node *node, contPathsElem *paths);

  // site Lib
  void loadSite(xml_node *node, siteLib *lib);
  void loadCfgInfo(xml_node *node, Site &site);
  void loadSramNameSp(xml_node *node, cfgInfoSite &cfgInfo);
  void loadSramName(xml_node *node, sramNameSpSite &sramNameSp);
  void loadCfgElem(xml_node *node, cfgElemsSite &cfgElems,
                   sramNameSpSite &sramNameSp);
  void loadFuncInCfg(xml_node *node, cfgElemSite &cfgElem,
                     sramNameSpSite &sramNameSp);
  void loadSramInFunc(xml_node *node, funcInCfgElemSite &funcInCfgElem,
                      sramNameSpSite &sramNameSp);

  // cluster Lib
  void loadClst(xml_node *node, clstLib *lib);
  void loadDimension(xml_node *node, Cluster &clst);
  void loadLinks(xml_node *node, Cluster &clst);
  void loadHrzORVtc(xml_node *node, dimenCluster &dim,
                    dimenCluster::direction dir);
  void loadSrcORSnk(xml_node *node, linkCluster &link,
                    linkCluster::pinType type);

  // tile Lib
  void loadTile(xml_node *node, tileLib *lib);
  void loadClstInTile(xml_node *node, Tile &tile);
  void loadSiteInClst(xml_node *node, clstTile &clst, Tile &tile);
  void loadSiteSram(xml_node *node, siteInTile &site, Tile &tile);
  void loadTransInTile(xml_node *node, Tile &tile);
  void loadSiteInTrans(xml_node *node, trnsTile &trans, Tile &tile);

  // bstrCMD
  void loadbstrCMD(xml_node *node);
};
} // namespace cil_lib
} // namespace FDU

#endif