#include "simpleArchWriter.h"
#include "arch/archlib.hpp"
#include "netlist.hpp"
#include "rutils.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <rapidxml/rapidxml_print.hpp>
#include <string>

namespace FDU {
namespace RT {
using namespace COS;
using namespace XML;
using namespace ARCH;
using namespace RT_CONST;

void SimpleArchWriter::writeArch() {
  Library *tileLib = FPGADesign::instance()->find_library("tile");
  for (Module *tileModule : tileLib->modules()) {
    TileWriter tileWriter(tileModule, path);
    tileWriter.writeTile();
  }
}

bool TileWriter::is_GRM_OR_GSB(const Instance *inst) const {
  return inst->module_type() == GRM || inst->module_type() == GSB;
}

void TileWriter::writeTile() {
  xml_node *root = create_element(&document(), "tileRef");
  set_attribute(root, "name", tile->name());
  xml_node *tilePins = create_element(root, "tilePins");
  for (Pin *tilePin : tile->pins()) {
    buildTilePin(tilePins, tilePin);
  }
  xml_node *sitePins = create_element(root, "sitePins");
  for (Instance *inst : tile->instances()) {
    if (is_GRM_OR_GSB(inst))
      continue;
    xml_node *instNode = create_element(sitePins, "site");
    set_attribute(instNode, "name", inst->name());
    for (Pin *sitePin : inst->pins()) {
      buildSitePin(instNode, sitePin);
    }
  }
  std::string filePath(path + tile->name() + ".xml");
  std::ofstream ofs(filePath.c_str());
  boost::filesystem::path p(filePath);
  assert(exists(p));
  ofs << document();
}

void TileWriter::buildTilePin(xml_node *parent, Pin *pin) {
  xml_node *tilePin = create_element(parent, "tilePin");
  set_attribute(tilePin, "name", pin->name());
  Net *net = pin->net();
  if (net) {
    for (Pin *netPin : net->pins()) {
      if (netPin == pin) {
        continue;
      }
      // there are pins DIRECTLY connected to slice such as FXINA3 of CENTER in
      // FDP500K. if we don't allow this, don't command out ASSERTD.
      // ASSERTD(is_GRM_OR_GSB(netPin->instance()), "tile pin connect to wrong
      // place(not opposite tile or GRM)");
      xml_node *tileLink = create_element(tilePin, "link");
      if (netPin->is_mpin()) {
        set_attribute(tileLink, "anotherTilePin", netPin->name());
      } else if (is_GRM_OR_GSB(netPin->instance())) {
        set_attribute(tileLink, "NetInGRM", netPin->down_pin()->net()->name());
      }
      // netPin is a logic pin, maybe.
      else {
        set_attribute(tileLink, "PinOfLogic", netPin->name());
      }
    }
  }
}

void TileWriter::buildSitePin(xml_node *parent, Pin *pin) {
  xml_node *pinNode = create_element(parent, "sitePin");
  set_attribute(pinNode, "name", pin->name());
  set_attribute(pinNode, "direction", pin->dir());
  Net *pinNet = pin->net();

  // 		ASSERTD(pinNet);
  if (pinNet) {
    for (Pin *netPin : pinNet->pins()) {
      if (netPin == pin) {
        continue;
      }
      // there are pins DIRECTLY connect to tile such as SOPIN of CENTER in
      // FDP500K. if we don't allow this, don't command out ASSERTD.
      // ASSERTD(is_GRM_OR_GSB(netPin->instance()));
      xml_node *siteLink = create_element(pinNode, "link");
      if (netPin->is_mpin()) {
        set_attribute(siteLink, "PinOfTile", netPin->name());
      } else if (is_GRM_OR_GSB(netPin->instance())) {
        set_attribute(siteLink, "NetInGrm", netPin->down_pin()->net()->name());
      } else {
        // we need COPY_VALUE because netPin->name() + "_OF_" +
        // netPin->owner()->name() just generate a temp string.
        set_attribute(siteLink, "PinOfAnotherSite",
                      netPin->name() + "_OF_" + netPin->owner()->name(),
                      COPY_VALUE);
      }
    }
  }
}

} // namespace RT
} // namespace FDU