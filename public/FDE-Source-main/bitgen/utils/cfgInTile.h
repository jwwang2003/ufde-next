#ifndef _CFGINTILE_H_
#define _CFGINTILE_H_

#include "utils/sizeSpan.h"

#include <iostream>
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// cfgFunc

struct cfgFunc {
  std::string _name;
  std::string _quomodo;
  std::string _manner;

  cfgFunc &operator=(const cfgFunc &t) {
    _name = t._name;
    _quomodo = t._quomodo;
    _manner = t._manner;
    return *this;
  }
};

inline std::ostream &operator<<(std::ostream &out, const cfgFunc &func) {
  out << func._name << "::" << func._quomodo << "::" << func._manner;
  return out;
}

//////////////////////////////////////////////////////////////////////////
// cfgElem

struct cfgElem {
  std::string _tileName;
  std::string _siteName;
  std::string _cfgElemName;
  cfgFunc _cfgElemFunc;
};
using vecCfgs = std::vector<cfgElem>;

//////////////////////////////////////////////////////////////////////////
// PIP in net

struct pipInfo {
  std::string _tileName;
  std::string _siteName;
  std::string _srcNet;
  std::string _snkNet;
};
using vecPips = std::vector<pipInfo>;

//////////////////////////////////////////////////////////////////////////
// sram bit

struct bitTile {
  enum TYPE { logic, route };

  TYPE _type;
  std::string _siteName;
  std::string _basicCell;
  std::string _sramName;
  std::string _ownerTile;
  std::string _landLordTile;
  std::string _cfgElemName;
  std::string _srcNet;
  std::string _snkNet;
  cfgFunc _cfgElemFunc;
  int _bitContent;
  sizeSpan _localPlace;
  sizeSpan _tileOffset;

  bool isLodger() const;
  bool hasOffset() const;
  bool isLogic() const;
  bool isRoute() const;
};

inline bool bitTile::isLodger() const { return _ownerTile != _landLordTile; }
inline bool bitTile::hasOffset() const { return _tileOffset != sizeSpan(0, 0); }
inline bool bitTile::isLogic() const { return _type == logic; }
inline bool bitTile::isRoute() const { return _type == route; }

using vecBits = std::vector<bitTile>;

//////////////////////////////////////////////////////////////////////////
// route & path

struct routeInfo {
  // "_siteName" will be used as tileName when
  // handle bidirectional pips in circuit
  // (adjustPips because "=-" in xdl)
  std::string _siteName;
  std::string _srcNet;
  std::string _snkNet;
};

struct pathInfo {
  std::string _bCellName;
  std::string _in;
  std::string _out;

  std::string toStr() const { return _in + " -> " + _out; }

  bool isEmpty() const {
    return _bCellName.empty() && _in.empty() && _out.empty();
  }
};

#endif