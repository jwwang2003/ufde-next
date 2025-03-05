#include "loadCil.h"
#include "PropertyName.h"
#include "cil/cilLib.h"
#include "exception/exceptions.h"
#include "log.h"
#include "main/arguments/Args.h"
#include "zfstream.h"

#include <boost/lexical_cast.hpp>
#include <fstream>
#include <rapidxml/rapidxml_utils.hpp>

namespace FDU {
namespace cil_lib {
using boost::lexical_cast;

cilLibrary *CilLibHandler::loadCil(xml_node *node) {
  ASSERT(_cilLib, "cilLib pointer is nullptr");
  _cilLib->setChipName(get_attribute(node, "name"));
  foreach_child(eleLibNode, node, "element_library") loadEleLib(eleLibNode);
  foreach_child(siteLibNode, node, "site_library") loadSiteLib(siteLibNode);
  foreach_child(clstLibNode, node, "cluster_library") loadClstLib(clstLibNode);
  foreach_child(transLibNode, node, "transmission_library")
      loadTrnsLib(transLibNode);
  foreach_child(tileLibNode, node, "tile_library") loadTileLib(tileLibNode);
  foreach_child(majorLibNode, node, "major_library") loadMajorLib(majorLibNode);
  foreach_child(bramLibNode, node, "bram_library") loadBramLib(bramLibNode);
  foreach_child(bstrcmdLibNode, node, "bstrcmd_library")
      loadbstrCMD(bstrcmdLibNode);
  return _cilLib;
}

void CilLibHandler::loadEleLib(xml_node *node) {
  foreach_child(eleNode, node, "element")
      loadElem(eleNode, _cilLib->getElemLib());
}

void CilLibHandler::loadElem(xml_node *node, elemLib *lib) {
  Element *ele = new Element(get_attribute(node, "name"));
  lib->addElement(ele);
  foreach_child(srams, node, "sram_info") loadSrams(srams, ele);
  foreach_child(paths, node, "path_info") loadPaths(paths, ele);
}

void CilLibHandler::loadSrams(xml_node *node, Element *ele) {
  foreach_child(sram, node, "sram") loadSram(sram, &ele->getSrams());
}

void CilLibHandler::loadPaths(xml_node *node, Element *ele) {
  foreach_child(path, node, "path") loadPath(path, &ele->getPaths());
}

void CilLibHandler::loadPath(xml_node *node, contPathsElem *paths) {
  string seg(get_attribute(node, "segregated"));
  bool segBool = false;
  if (seg[0] != 'n')
    segBool = true;
  pathElem *path = new pathElem("", get_attribute(node, "in"),
                                get_attribute(node, "out"), segBool);
  paths->addPath(path);

  foreach_child(cfgs, node, "configuration_info") {
    foreach_child(cfg, cfgs, "sram") loadSram(cfg, &path->getCfgInfo());
  }
}

void CilLibHandler::loadSram(xml_node *node, contSramsElem *srams) {
  string name(get_attribute(node, "name"));
  string dft(get_attribute(node, "default"));
  if (dft == "") {
    int ctt = stoi(get_attribute(node, "content"));
    srams->addSram(new sramElem(false, ctt, name));
  } else {
    int ctt = stoi(dft);
    srams->addSram(new sramElem(true, ctt, name));
  }
}

void CilLibHandler::loadSiteLib(xml_node *node) {
  foreach_child(siteNode, node, "primitive_site")
      loadSite(siteNode, _cilLib->getSiteLib());
  // for new name block_site
  foreach_child(siteNode, node, "block_site")
      loadSite(siteNode, _cilLib->getSiteLib());
}

void CilLibHandler::loadSite(xml_node *node, siteLib *lib) {
  Site &site = *lib->addSite(
      new Site(get_attribute(node, "name"), _cilLib->getElemLib()));
  foreach_child(cfgInfoNode, node, "config_info")
      loadCfgInfo(cfgInfoNode, site);
}

void CilLibHandler::loadCfgInfo(xml_node *node, Site &site) {
  cfgInfoSite &cfgInfo = site.getCfgInfo();
  foreach_child(sramNameSpNode, node, "sram_namespace")
      loadSramNameSp(sramNameSpNode, cfgInfo);
  foreach_child(cfgElemNode, node, "cfg_element")
      loadCfgElem(cfgElemNode, cfgInfo.getCfgElems(), cfgInfo.getSramNameSp());
}

void CilLibHandler::loadSramNameSp(xml_node *node, cfgInfoSite &cfgInfo) {
  foreach_child(sramNameNode, node, "sram_name")
      loadSramName(sramNameNode, cfgInfo.getSramNameSp());
}

void CilLibHandler::loadSramName(xml_node *node, sramNameSpSite &sramNameSp) {
  sramNameInNameSpSite &sramName =
      *sramNameSp.addSramName(new sramNameInNameSpSite(
          get_attribute(node, "name"), _cilLib->getElemLib()));
  foreach_child(bCellNode, node, "basic_cell") {
    sramName.addBasicCell(new bCellInSramNameSite(
        get_attribute(bCellNode, "name"), get_attribute(bCellNode, "sram"),
        _cilLib->getElemLib()));
  }
}

void CilLibHandler::loadCfgElem(xml_node *node, cfgElemsSite &cfgElems,
                                sramNameSpSite &sramNameSp) {
  cfgElemSite &cfgElem = *cfgElems.addCfgElem(new cfgElemSite(
      get_attribute(node, "name"), _cilLib->getElemLib(), &sramNameSp));
  foreach_child(funcInCfgNode, node, "function")
      loadFuncInCfg(funcInCfgNode, cfgElem, sramNameSp);
}

void CilLibHandler::loadFuncInCfg(xml_node *node, cfgElemSite &cfgElem,
                                  sramNameSpSite &sramNameSp) {
  string dft(get_attribute(node, "default"));
  bool boolDft = dft.at(0) == 'y' ? true : false;
  string quomodo(get_attribute(node, "quomodo"));
  quomodo = quomodo == "" ? "naming" : quomodo;
  string manner(get_attribute(node, "manner"));
  manner = manner == "" ? "enumeration" : manner;

  funcInCfgElemSite &funcInCfg = *cfgElem.addFunc(
      new funcInCfgElemSite(get_attribute(node, "name"), quomodo, manner,
                            boolDft, _cilLib->getElemLib(), &sramNameSp));
  foreach_child(sramInFuncNode, node, "sram")
      loadSramInFunc(sramInFuncNode, funcInCfg, sramNameSp);
}

void CilLibHandler::loadSramInFunc(xml_node *node,
                                   funcInCfgElemSite &funcInCfgElem,
                                   sramNameSpSite &sramNameSp) {
  string ctt(get_attribute(node, "content"));
  int content = ctt == "" ? 0 : stoi(ctt);
  string addr(get_attribute(node, "address"));
  int optAddr = addr == "" ? -1 : stoi(addr);

  funcInCfgElem.addSram(new sramInCfgElemSite(
      get_attribute(node, "basic_cell"), get_attribute(node, "name"), content,
      optAddr, _cilLib->getElemLib(), &sramNameSp));
}

void CilLibHandler::loadClstLib(xml_node *node) {
  foreach_child(clstNode, node, "homogeneous_cluster")
      loadClst(clstNode, _cilLib->getClstLib());
}

void CilLibHandler::loadClst(xml_node *node, clstLib *lib) {
  Cluster &clst = *lib->addCluster(new Cluster(get_attribute(node, "name"),
                                               get_attribute(node, "type"),
                                               _cilLib->getSiteLib()));
  foreach_child(dimensionNode, node, "dimension")
      loadDimension(dimensionNode, clst);
  foreach_child(linkNode, node, "link_info") loadLinks(linkNode, clst);
}

void CilLibHandler::loadDimension(xml_node *node, Cluster &clst) {
  dimenCluster &dim = clst.getDimension();
  xml_node *hrzDimenNode = node->first_node("horizontal_dimension");
  loadHrzORVtc(hrzDimenNode, dim, dimenCluster::horizontal);
  xml_node *vtcDimenNode = node->first_node("vertical_dimension");
  loadHrzORVtc(vtcDimenNode, dim, dimenCluster::vertical);
}

void CilLibHandler::loadHrzORVtc(xml_node *node, dimenCluster &dim,
                                 dimenCluster::direction dir) {
  dim.setSizeByDirect(dir, stoi(get_attribute(node, "cluster_size")),
                      stoi(get_attribute(node, "norm_size")));
}
void CilLibHandler::loadLinks(xml_node *node, Cluster &clst) {
  contLinksCluster &links = clst.getLinks();
  foreach_child(linkNode, node, "link") {
    linkCluster &link = *links.addLink(new linkCluster(
        clst.getRefSiteName(), _cilLib->getSiteLib(),
        lexical_cast<sizeSpan>(get_attribute(linkNode, "container_size"))));
    xml_node *srcNode = linkNode->first_node("source");
    loadSrcORSnk(srcNode, link, linkCluster::source);
    xml_node *snkNode = linkNode->first_node("destination");
    loadSrcORSnk(snkNode, link, linkCluster::sink);
  }
}

void CilLibHandler::loadSrcORSnk(xml_node *node, linkCluster &link,
                                 linkCluster::pinType type) {
  link.setSrcOrSnk(type, get_attribute(node, "pin_name"),
                   stoi(get_attribute(node, "row")),
                   stoi(get_attribute(node, "column")));
}

void CilLibHandler::loadTrnsLib(xml_node *node) {
  foreach_child(transNode, node, "signal_transmission") {
    _cilLib->getTrnsLib()->addTrans(new Trans(get_attribute(transNode, "name"),
                                              get_attribute(transNode, "type"),
                                              _cilLib->getSiteLib()));
  }
}

void CilLibHandler::loadTileLib(xml_node *node) {
  foreach_child(tileNode, node, "tile")
      loadTile(tileNode, _cilLib->getTileLib());
}

void CilLibHandler::loadTile(xml_node *node, tileLib *lib) {
  Tile &tile = *lib->addTile(
      new Tile(get_attribute(node, "name"),
               lexical_cast<sizeSpan>(get_attribute(node, "sram_amount")),
               _cilLib->getClstLib(), _cilLib->getTrnsLib()));
  xml_node *clstsNode = node->first_node("cluster_info");
  xml_node *transsNode = node->first_node("transmission_info");
  foreach_child(clstNode, clstsNode, "cluster") loadClstInTile(clstNode, tile);
  foreach_child(transNode, transsNode, "transmission")
      loadTransInTile(transNode, tile);
}

void CilLibHandler::loadClstInTile(xml_node *node, Tile &tile) {
  string location(get_attribute(node, "location"));
  sizeSpan lct =
      location == "" ? sizeSpan(-1, -1) : lexical_cast<sizeSpan>(location);
  clstTile &clst = *tile.getClsts().addClst(new clstTile(
      get_attribute(node, "type"), lct, tile.getName(), _cilLib->getClstLib()));
  foreach_child(siteClstNode, node, "site")
      loadSiteInClst(siteClstNode, clst, tile);
}

void CilLibHandler::loadTransInTile(xml_node *node, Tile &tile) {
  string location(get_attribute(node, "location"));
  sizeSpan lct =
      location == "" ? sizeSpan(-1, -1) : lexical_cast<sizeSpan>(location);

  trnsTile &trans = *tile.getTrnss().addTrns(new trnsTile(
      get_attribute(node, "type"), lct, tile.getName(), _cilLib->getTrnsLib()));
  foreach_child(siteTransNode, node, "site")
      loadSiteInTrans(siteTransNode, trans, tile);
}

void CilLibHandler::loadSiteInClst(xml_node *node, clstTile &clst, Tile &tile) {
  string pos = get_attribute(node, "position");
  sizeSpan pst =
      pos == ""
          ? sizeSpan(-1, -1)
          : lexical_cast<sizeSpan>(pos); // VCC does not have a position info.
  siteInTile &site = *clst.addSite(
      new siteInTile(get_attribute(node, "name"), clst.getRefSiteName(),
                     clst.getRefTileName(), pst, _cilLib->getSiteLib()));
  foreach_child(siteSramNode, node, "site_sram")
      loadSiteSram(siteSramNode, site, tile);
}

void CilLibHandler::loadSiteInTrans(xml_node *node, trnsTile &trans,
                                    Tile &tile) {
  string pos(get_attribute(node, "position"));
  sizeSpan pst = pos == "" ? sizeSpan(-1, -1) : lexical_cast<sizeSpan>(pos);
  siteInTile &site = *trans.addSite(
      new siteInTile(get_attribute(node, "name"), trans.getRefSiteName(),
                     trans.getRefTileName(), pst, _cilLib->getSiteLib()));
  foreach_child(siteSramNode, node, "site_sram")
      loadSiteSram(siteSramNode, site, tile);
}

// #define CHECK_SRAM_REDEFINE
void CilLibHandler::loadSiteSram(xml_node *node, siteInTile &site, Tile &tile) {
  foreach_child(sramNode, node, "sram") {
    std::string offset(get_attribute(sramNode, "brick_offset"));
    sizeSpan off =
        offset == "" ? sizeSpan(0, 0) : lexical_cast<sizeSpan>(offset);
    sizeSpan localPlace =
        lexical_cast<sizeSpan>(get_attribute(sramNode, "local_place"));
    sramInSiteSramTile *newSram = new sramInSiteSramTile(
        get_attribute(sramNode, "sram_name"),
        get_attribute(sramNode, "basic_cell"), localPlace, off,
        get_attribute(sramNode, "owner_tile"),
        /*tile.getName()*/ "", site.getRefSiteName(), _cilLib->getSiteLib());
#ifdef CHECK_SRAM_REDEFINE
    if (site.hasSram(*newSram)) {
      FDU_LOG(WARN) << newSram->getBasicCell() + " : " + newSram->getName()
                    << " sram redefined in tile " << tile.getName();
      delete newSram;
    } else {
#endif

      site.getSiteSram().addSram(newSram);
#ifdef CHECK_SRAM_REDEFINE
    }
#endif
  }
}

void CilLibHandler::loadMajorLib(xml_node *node) {
  foreach_child(majorNode, node, "major") {
    _cilLib->getMajorLib()->addMajor(
        new Major(stoi(get_attribute(majorNode, "address")),
                  stoi(get_attribute(majorNode, "frm_amount")),
                  stoi(get_attribute(majorNode, "tile_col"))));
  }
}

void CilLibHandler::loadBramLib(xml_node *node) {
  foreach_child(bramNode, node, "bram") {
    _cilLib->getBramLib()->addBram(new Bram(
        stoi(get_attribute(bramNode, "address")),
        stoi(get_attribute(bramNode, "bl")), get_attribute(bramNode, "type")));
  }
}

void CilLibHandler::loadbstrCMD(xml_node *node) {
  foreach_child(cmdNode, node, "parameter") {
    _cilLib->getbstrCMDLib()->addbstrParameter(
        new bstrParameter(get_attribute(cmdNode, "name"),
                          std::stoi(get_attribute(cmdNode, "value"))));
  }
  foreach_child(cmdNode, node, "command") {
    _cilLib->getbstrCMDLib()->addbstrCMD(new bstrCMD(
        get_attribute(cmdNode, "cmd"), get_attribute(cmdNode, "parameter")));
  }
}

} // namespace cil_lib
} // namespace FDU

namespace FDU {
namespace cil_lib {
using namespace FDU::XML;

using xml_file = rapidxml::file<>;

cilLibrary *cilLibrary::loadCilLib(const std::string &file) {
  cilLibrary *lib = new cilLibrary();
  CilLibHandler cilh{lib};
  xml_document doc;
  ifstrm ifs(file.c_str());
  xml_file xfile(ifs);
  doc.parse<0>(xfile.data());
  cilh.loadCil(doc.first_node());
  return lib;
}
} // namespace cil_lib
} // namespace FDU
