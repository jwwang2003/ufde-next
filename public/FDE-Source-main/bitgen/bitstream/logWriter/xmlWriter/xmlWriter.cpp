#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include "log.h"
#include "utils.h"
#include "xmlWriter.h"

namespace BitGen {
namespace bitstream {
namespace xmlio {
using boost::lexical_cast;
using namespace boost::adaptors;
using std::string;

void xmlWriter::write(const std::string &fileName) {
  _ofs.open(fileName.c_str());
  ASSERT(_ofs, "xmlWriter: can not open..." + fileName);
  create_declaration("1.0");
  /*		set_attribute(&document(), "encoding", "UTF-8");*/
  writeTile();
  _ofs << document();
  _ofs.close();
}

void xmlWriter::writeTile() {
  switch (_mode) {
  case NORMAL:
    writeNormal();
    break;
  case OVERLAPS:
    writeOverlaps();
    break;
  default:
    throw BstrException("xmlWriter: invalid mode for me");
  }
}

void xmlWriter::writeNormal() {
  const vecBits &tileBits = _tile.getBits();
  xml_node *root = create_element(&document(), "tile");
  set_attribute(root, "name", _tile.getName(), COPY_VALUE);
  set_attribute(root, "amount", to_string(tileBits.size()), COPY_VALUE);

  xml_node *logicNodes = create_element(root, "logic");
  auto logicAmount =
      std::count_if(tileBits.begin(), tileBits.end(),
                    [](const bitTile &bit) { return bit.isLogic(); });
  set_attribute(logicNodes, "amount", to_string(logicAmount), COPY_VALUE);

  xml_node *routeNodes = create_element(root, "route");
  auto routeAmount =
      std::count_if(tileBits.begin(), tileBits.end(),
                    [](const bitTile &bit) { return bit.isRoute(); });
  set_attribute(routeNodes, "amount", to_string(routeAmount), COPY_VALUE);
  ASSERT(tileBits.size() == logicAmount + routeAmount,
         "xmlWriter: some invalid bits exist in this tile..." +
             _tile.getName());

  for (const bitTile &bit : tileBits) {
    if (bit.isLogic())
      writeBit(bit, logicNodes);
    else
      writeBit(bit, routeNodes);
  }
}

void xmlWriter::writeOverlaps() {
  xml_node *root = create_element(&document(), "tile");
  set_attribute(root, "name", _tile.getName(), COPY_VALUE);
  const dfotTileBstr::Overlaps &overlaps = _tile.getOverlaps();
  int groups = 0;
  for (const std::vector<bitTile *> &vec : overlaps)
    if (vec.size() > 1u) {
      xml_node *overlapNode = create_element(root, "overlaps");
      set_attribute(overlapNode, "amount", to_string(vec.size()), COPY_VALUE);
      for (const bitTile *pBit : vec)
        writeBit(*pBit, overlapNode);
      ++groups;
    }
  set_attribute(root, "amount", to_string(groups), COPY_VALUE);
}

void xmlWriter::writeBit(const bitTile &bit, xml_node *node) {
  xml_node *bitNode = create_element(node, "bit");
  if (!bit._ownerTile.empty())
    set_attribute(bitNode, "owner", bit._ownerTile);
  if (!bit._landLordTile.empty())
    set_attribute(bitNode, "landlord", bit._landLordTile);

  set_attribute(bitNode, "local_place", to_string(bit._localPlace), COPY_VALUE);
  set_attribute(bitNode, "tile_offset", to_string(bit._tileOffset), COPY_VALUE);
  set_attribute(bitNode, "site", bit._siteName);
  if (bit.isLogic()) {
    set_attribute(bitNode, "cfg_elem", bit._cfgElemName);
    set_attribute(bitNode, "function", to_string(bit._cfgElemFunc), COPY_VALUE);
  } else {
    set_attribute(bitNode, "src_net", bit._srcNet);
    set_attribute(bitNode, "snk_net", bit._snkNet);
  }
  set_attribute(bitNode, "basic_cell", bit._basicCell);
  set_attribute(bitNode, "sram", bit._sramName);
  set_attribute(bitNode, "value", to_string(bit._bitContent), COPY_VALUE);
}
} // namespace xmlio
} // namespace bitstream
} // namespace BitGen