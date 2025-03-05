#include "circuit/netLib/net/pipInNet.h"
#include "exception/exceptions.h"
#include "log.h"
#include "utils/sizeSpan.h"
#include "utils/specialNets.h"

namespace BitGen {
namespace circuit {
using namespace boost;

void pipNet::constructFromXDL(const std::string &info) {
  static const char *pipExp =
      // possibly leading whitespace:
      "\\s*"
      // pip information: [1] = tile, [2] = wireL, [3] = dir, [4] = wireR
      "(.*?)\\s+(.*?)\\s*([-=>]+)\\s*(.*?)\\s*";
  static boost::regex pipRegex(pipExp);

  boost::smatch match;
  ASSERT(boost::regex_match(info, match, pipRegex),
         "pip: invalid pip info in xdl ... " + info);
  _tile = match[1].str();
  _wireL = match[2].str();
  _dir = match[3].str();
  _wireR = match[4].str();
  _name = _wireL + _dir + _wireR;

  if (_wireL.substr(0, 4) == "TBUF" && _wireR.substr(0, 4) == "TBUF" &&
      _dir == "=-")
    _dir = "->";
}

void pipNet::adjustBRAMTile(int offset) {
  if (offset) {
    // used for fdp3000k-pq208
    if (_tile.substr(0, 4) == "BRAM") {
      _tile.erase(0, 4);
      int row, col;
      sizeSpan::extractCoordinate(_tile.c_str(), 'R', row, 'C', col);
      row += offset;
      std::ostringstream tStream;
      tStream << "EMPTYR" << row << "C" << col;
      _tile = tStream.str();
    }
    // used for fdp3000k-cb228
    else if (_tile.substr(0, 5) == "LBRAM" || _tile.substr(0, 5) == "RBRAM") {
      _tile = _tile.substr(0, 1) + "EMPTY" + _tile.substr(5) +
              "C0"; // "C0" for dummy
      int row, col;
      sizeSpan::extractCoordinate(_tile.substr(6).c_str(), 'R', row, 'C', col);
      row += offset;
      std::ostringstream tStream;
      tStream << _tile.substr(0, 7) << row;
      _tile = tStream.str();
    }
  }
}

void pipNet::adjustBRAMPip() {
  if (isBRAMPip()) {
    int wireLOff = getSpecialNetOffset(_wireL);
    int wireROff = getSpecialNetOffset(_wireR);
    int wireOff = 0;
    if (wireLOff == 0 && wireROff != 0)
      wireOff = wireROff;
    else if (wireLOff != 0 && wireROff == 0)
      wireOff = wireLOff;
    else if (wireLOff != 0 && wireROff != 0) {
      ASSERTD(wireLOff == wireROff, "circuit: invalid BRAM pip ... " + _tile +
                                        ": " + _wireL + "->" + _wireR);
      wireOff = wireLOff;
    } else
      wireOff = 0;

    adjustBRAMTile(wireOff);
  }
}

bool contPipsNet::hasBiDirPip() const {
  return find_if(pips(), [](const pipNet *pip) { return pip->isBiDirPip(); }) !=
         _pips.end();
}

bool contPipsNet::hasBRAMPip() const {
  return find_if(pips(), [](const pipNet *pip) { return pip->isBRAMPip(); }) !=
         _pips.end();
}

void contPipsNet::adjustBRAMPips() {
  if (hasBRAMPip())
    for (pipNet *pip : pips())
      pip->adjustBRAMPip();
}

void pipNet::saveTo(vecPips &vPips) {
  pipInfo tPip;
  tPip._tileName = _tile;
  tPip._siteName = "";
  tPip._srcNet = _wireL;
  tPip._snkNet = _wireR;
  vPips.push_back(tPip);
}

void contPipsNet::listPips(vecPips &vPips) {
  for (pipNet *pip : pips())
    pip->saveTo(vPips);
}

void contPipsNet::listPotentialDrivers(std::string cktTileName,
                                       std::string wireName,
                                       std::vector<routeInfo> &drivers) {
  // self is also a driver
  routeInfo self;
  self._siteName = cktTileName;
  self._srcNet = wireName;
  self._snkNet = wireName;
  drivers.push_back(self);

  int wireType = getWireType(wireName);
  addDrivers(cktTileName, wireName, wireType, drivers);
}

static auto matchRoute(const string &cktTileName, const string &wireName) {
  return [&](const routeInfo &rt) {
    return rt._siteName == cktTileName && rt._srcNet == wireName;
  };
}

// need to test
string contPipsNet::getDriveAbove(pipNet &curPip, vector<routeInfo> &drivers) {
  string beDrivedNet = "";
  // judge driver upward
  pipNetIter pipIt = boost::find(pips(), &curPip);
  for (pipIt = pipIt == pips().begin() ? pipIt : pipIt - 1;
       pipIt >= pips().begin(); --pipIt) {
    auto driverIt =
        std::find_if(drivers.begin(), drivers.end(),
                     matchRoute(pipIt->getTile(), pipIt->getWireR()));
    if (driverIt != drivers.end()) {
      beDrivedNet = driverIt->_snkNet;
      break;
    }

    if (pipIt == pips().begin())
      break;
  }

  if (beDrivedNet.empty()) {
    // judge driver downward according to wireR
    FDU_LOG(WARN) << Warning("circuit: pip \"" + curPip.getWireL() +
                             curPip.getDir() + curPip.getWireR() +
                             "\" can not be judged upward, try to judge "
                             "downward acccrding to wireR");
    for (pipNetIter pipIt = boost::find(pips(), &curPip) + 1;
         pipIt < pips().end(); ++pipIt) {
      auto driverIt =
          std::find_if(drivers.begin(), drivers.end(),
                       matchRoute(pipIt->getTile(), pipIt->getWireR()));
      if (driverIt != drivers.end()) {
        beDrivedNet = driverIt->_snkNet;
        break;
      }
    }
  }

  if (beDrivedNet.empty()) {
    // judge driver downward according to wireL
    FDU_LOG(WARN) << Warning("circuit: pip \"" + curPip.getWireL() +
                             curPip.getDir() + curPip.getWireR() +
                             "\" can not be judged upward, try to judge "
                             "downward acccrding to wireL");
    for (pipNetIter pipIt = boost::find(pips(), &curPip) + 1;
         pipIt < pips().end(); ++pipIt) {
      if (pipIt->getDir() == "=-") {
        auto driverIt =
            std::find_if(drivers.begin(), drivers.end(),
                         matchRoute(pipIt->getTile(), pipIt->getWireL()));
        if (driverIt != drivers.end()) {
          beDrivedNet = driverIt->_snkNet;
          break;
        }
      }
    }
  }

  return beDrivedNet;
}

void contPipsNet::adjustEachBiDirPip(pipNet &pip) {
  if (pip.isBiDirPip()) {
    string cktTileName = pip.getTile();
    string wireL = pip.getWireL();
    string wireR = pip.getWireR();

    vector<routeInfo> drivers;
    listPotentialDrivers(cktTileName, wireL, drivers);
    listPotentialDrivers(cktTileName, wireR, drivers);
    string beDrivedNet = getDriveAbove(pip, drivers);
    if (!beDrivedNet.empty()) {
      if (beDrivedNet == wireL) {
      } else if (beDrivedNet == wireR) {
        pip.swapWires();
      } else
        throw CktException("can't judge path " + cktTileName + ":" + wireL +
                           " =- " + wireR + " 's direction");
    } else
      throw CktException("can't judge path " + cktTileName + ":" + wireL +
                         " =- " + wireR + " 's direction");
  }
}

void contPipsNet::adjustBiDirPips() {
  if (hasBiDirPip())
    for (pipNet *pip : pips())
      adjustEachBiDirPip(*pip);
}

} // namespace circuit
} // namespace BitGen