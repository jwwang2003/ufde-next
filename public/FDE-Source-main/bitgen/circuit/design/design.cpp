#include "circuit/design/design.h"
#include "main/arguments/Args.h"

namespace BitGen {
namespace circuit {

//////////////////////////////////////////////////////////////////////////
// static data members

//	const char* Design::_FDP1000K    = "fdp1000k";
//	const char* Design::_1000K_PKG   = "pq208";
const char *Design::_FDP_SPEED = "-6";
const char *Design::_NCD_Version = "v3.1";

void Design::constructFromXDL(cktBase::xdlMatch *match) {
  _cktName = ((*match)[1]).str();
  _devName = ((*match)[2]).str();
  _pkgName = ((*match)[3]).str();
  _speed = ((*match)[5]).str();
  _ncdVer = ((*match)[6]).str();

  delete match;
}

void Design::constructFromXML(const std::string &cktName) {
  _cktName = cktName;
  _devName = args._device;
  _pkgName = args._package;
  _speed = _FDP_SPEED;
  _ncdVer = _NCD_Version;
}

} // namespace circuit
} // namespace BitGen