#ifndef _XMLWRITER_H_
#define _XMLWRITER_H_

#include "bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/dfotTileBstr.h"
#include "cfgInTile.h"
#include "xmlutils.h"
#include <fstream>

namespace BitGen {
namespace bitstream {
namespace xmlio {
using namespace FDU::XML;

class xmlWriter : public DomBuilder {
public:
  enum MODE { NORMAL, OVERLAPS };
  explicit xmlWriter(MODE mode, const dfotTileBstr &tile)
      : DomBuilder(), _mode(mode), _tile(tile) {}
  void write(const std::string &fileName);

private:
  void writeTile();
  void writeNormal();
  void writeOverlaps();
  void writeBit(const bitTile &bit, xml_node *node);

private:
  const dfotTileBstr &_tile;
  MODE _mode;
  std::ofstream _ofs;
};
} // namespace xmlio
} // namespace bitstream
} // namespace BitGen

#endif