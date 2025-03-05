#include "circuit/xdl/xdlCktHandler.h"
#include "exception/exceptions.h"
#include "log.h"
#include "main/arguments/Args.h"
#include "zfstream.h"

#include <fstream>

namespace BitGen {
namespace circuit {
//////////////////////////////////////////////////////////////////////////
// load XDL file to string

void xdlCktHandler::loadXDL(std::istream &xdl) {
  ASSERT(xdl, "xdl: can not open xdl netlist");

  _contents.erase();
  _contents.reserve(xdl.rdbuf()->in_avail());
  char c;
  while (xdl.get(c)) {
    if (_contents.capacity() == _contents.size())
      _contents.reserve(_contents.capacity() * 3);
    _contents.append(1, c);
  }

  regulateContents();
}

void xdlCktHandler::regulateContents() {
  static const char *redundancyExp = "\\s+\\n*:";
  static const char *regulateExp = ":";
  static boost::regex redundancyRegex(redundancyExp);

  ostringstream oss;
  std::ostream_iterator<char, char> out(oss);
  boost::regex_replace(out, _contents.begin(), _contents.end(), redundancyRegex,
                       regulateExp);

  _contents = oss.str();
}

//////////////////////////////////////////////////////////////////////////
// parse

void xdlCktHandler::parse(const std::string &file) {
  FDU::ifstrm ifs(file.c_str());
  loadXDL(ifs);

  // Object regex: design, inst, net
  static const char *objectExp =
      // possibly leading whitespace:
      "\\s*"
      // object information: [1] = ObjName, [2] = ObjInfo
      "(^[designinstnet]+)\\s*(.*?)\\s*\\n*;";
  static boost::regex objRegex(objectExp);

  boost::sregex_iterator it(_contents.begin(), _contents.end(), objRegex);
  boost::sregex_iterator end;
  std::for_each(it, end, parseObject(_curCkt));

  // release
  _contents.clear();
}

//////////////////////////////////////////////////////////////////////////
// function object for parsing

bool parseObject::operator()(const cktBase::xdlMatch &what) const {
  // design regex
  static const char *dsignExp =
      // possibly leading whitespace:
      "\\s*"
      // design information: [1] = cktName, [2] = devName, [3] = pkgName, [5] =
      // speed, [6] = ncdVersion
      //			"\"(.*?)\"\\s*(.*?)([pq]+\\d+)(.*?)\\s*([vV]\\d*\\.\\d*)\\s*.*";
      "\"(.*?)\"\\s*(.*?)((pq|cb|fg)\\d+)(.*?)\\s*([vV]\\d*\\.\\d*)\\s*.*";
  static boost::regex dsignRegex(dsignExp);

  // inst regex
  static const char *instExp =
      // possibly leading whitespace:
      "\\s*"
      // inst information: [1] = instName, [2] = instType, [3] = spot, [4] =
      // instPos, [5] = siteOfInst
      "\"(.*?)\"\\s*\"(.*?)\"\\s*,\\s*([a-zA-Z]+)\\s*([a-zA-Z0-9]+)\\s*(.*?)"
      "\\s*,\\s*\\n*"
      // cfg information: [6] = cfgsOfInst
      "\\s*cfg\\s*\"\\s*(.*?)\\s*\\n*\"\\s*";
  static boost::regex instRegex(instExp);

  // net regex
  static const char *netExp =
      // possibly leading whitespace:
      "\\s*"
      // net information: [1] = netName
      //			"\"(.*?)\"\\s*.*?\\s*,\\s*.*?\\s*\\n*"
      "\"(.*?)\"\\s*.*?\\s*,\\s*.*?\\s*\\n*"
      // pin & pip information: [2] = Pins & Pips
      "\\s*(.*?)\\s*";
  static boost::regex netRegex(netExp);

  cktBase::xdlMatch *match = new cktBase::xdlMatch;
  string objName = what[1].str();
  string objInfo = what[2].str();
  if (objName == "design") {
    ASSERT(boost::regex_match(objInfo, *match, dsignRegex),
           "design: invalid design info in xdl");
    _curCkt->getDesign()->constructFromXDL(match);
  } else if (objName == "inst") {
    ASSERT(boost::regex_match(objInfo, *match, instRegex),
           "inst: invalid inst info in xdl");
    _curCkt->getInsts()->addInst(new Inst(match));
  } else if (objName == "net") {
    ASSERT(boost::regex_match(objInfo, *match, netRegex),
           "net: invalid net info in xdl");
    _curCkt->getNets()->addNet(new Net(match, _curCkt->getInsts()));
  } else
    throw CktException("objName: invalid object name in xdl ... " + objName);

  return true;
}

} // namespace circuit
} // namespace BitGen