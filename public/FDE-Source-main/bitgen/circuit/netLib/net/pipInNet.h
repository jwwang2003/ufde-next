#ifndef _PIPINNET_H_
#define _PIPINNET_H_

#include "circuit/cktBase.h"
#include "container/Container.h"
#include "utils/cfgInTile.h"
#include "utils/specialNets.h"

namespace BitGen {
namespace circuit {

class pipNet : public cktBase {
private:
  std::string _tile;
  std::string _wireL;
  std::string _wireR;
  std::string _dir;

public:
  explicit pipNet(const std::string &info) : cktBase("", nullptr) {
    constructFromXDL(info);
  }

  pipNet(const std::string &tile, const std::string &wireL,
         const std::string &wireR, const std::string &dir)
      : _tile(tile), _wireL(wireL), _wireR(wireR), _dir(dir) {}

  //		bool operator == (const pipNet&) const;

  bool isBiDirPip() const;
  bool isBRAMPip() const;
  void adjustBRAMTile(int offset);
  void adjustBRAMPip();
  int getSpecialNetOffset(const std::string &wire);

  string getTile() const { return _tile; }
  string getWireL() const { return _wireL; }
  string getWireR() const { return _wireR; }
  string getDir() const { return _dir; }
  void swapWires() { swap(_wireL, _wireR); }

  void saveTo(vecPips &vPips);

  virtual void constructFromXDL(const std::string &info);
  virtual void constructFromXML() {}
};

class contPipsNet {
public:
  using pipsNetType = cktContainer<pipNet>::range_type;
  using const_pipsNetType = cktContainer<pipNet>::const_range_type;
  using pipNetIter = cktContainer<pipNet>::iterator;
  using const_pipNetIter = cktContainer<pipNet>::const_iterator;

private:
  cktContainer<pipNet> _pips;

public:
  pipsNetType pips() { return _pips.range(); }
  const_pipsNetType pips() const { return _pips.range(); }

  pipNet *addPip(pipNet *pip) { return _pips.add(pip); }
  pipNet &getPip(const std::string &pipName) { return *pips().find(pipName); }

  bool hasBRAMPip() const;
  void adjustBRAMPips();

  bool hasBiDirPip() const;
  void listPotentialDrivers(std::string cktTileName, std::string wireName,
                            std::vector<routeInfo> &drivers);
  string getDriveAbove(pipNet &curPip, vector<routeInfo> &drivers);
  void adjustEachBiDirPip(pipNet &pip);
  void adjustBiDirPips();

  void listPips(vecPips &vPips);
};

inline bool pipNet::isBiDirPip() const { return _dir == "=-"; }

inline bool pipNet::isBRAMPip() const {
  return // used for fdp3000k-pq208
      _tile.substr(0, 4) == "BRAM" ||
      // used for fdp3000k-cb228
      _tile.substr(0, 5) == "LBRAM" || _tile.substr(0, 5) == "RBRAM";
}

inline int pipNet::getSpecialNetOffset(const std::string &wire) {
  std::string::size_type underlinePos = wire.find_first_of("_");
  std::string net = wire.substr(underlinePos + 1);
  return SPECIAL_NETS.count(net) ? SPECIAL_NETS[net] : 0;
}

inline bool operator==(const pipNet &lhs, const pipNet &rhs) {
  return lhs.getTile() == rhs.getTile() && lhs.getWireL() == rhs.getWireL() &&
         lhs.getWireR() == rhs.getWireR() && lhs.getDir() == rhs.getDir();
}

} // namespace circuit
} // namespace BitGen

#endif