#ifndef _POSININST_H_
#define _POSININST_H_

#include <string>

namespace BitGen {
namespace circuit {

struct posInst {
  std::string _spot;
  std::string _tile;
  std::string _site;

  void constructFromXDL(const std::string &spot, const std::string &tile,
                        const std::string &site);
  void constructFromXML(const std::string &spot, const std::string &tile,
                        const std::string &site) {
    constructFromXDL(spot, tile, site);
  }
  void format();
};

} // namespace circuit
} // namespace BitGen

#endif