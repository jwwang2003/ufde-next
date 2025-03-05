#ifndef _DESIGN_H_
#define _DESIGN_H_

#include "circuit/cktBase.h"
#include <string>

namespace BitGen {
namespace circuit {

struct Design {
  //		static const char* _FDP1000K;
  //		static const char* _1000K_PKG;
  static const char *_FDP_SPEED;
  static const char *_NCD_Version;

  std::string _cktName;
  std::string _devName;
  std::string _pkgName;
  std::string _speed;
  std::string _ncdVer;

  void constructFromXDL(cktBase::xdlMatch *match);
  void constructFromXML(const std::string &cktName);
};

} // namespace circuit
} // namespace BitGen

#endif