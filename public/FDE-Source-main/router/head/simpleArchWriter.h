#ifndef _SIMPLEARCHWRITER_H
#define _SIMPLEARCHWRITER_H

#include "netlist.hpp"
#include "xmlutils.h"

namespace FDU {
namespace RT {
using namespace XML;
using namespace COS;

class TileWriter : public DomBuilder {
public:
  TileWriter(Module *tileModule, const string &_path)
      : DomBuilder(), tile(tileModule), path(_path) {}
  void writeTile();
  void buildTilePin(xml_node *parent, Pin *pin);
  void buildSitePin(xml_node *parent, Pin *pin);

private:
  bool is_GRM_OR_GSB(const Instance *inst) const;
  Module *tile;
  string path;
};

class SimpleArchWriter {
public:
  SimpleArchWriter(const string &_path = "rrg_log\\") : path(_path) {}
  void writeArch();

private:
  string path;
};
} // namespace RT
} // namespace FDU

#endif