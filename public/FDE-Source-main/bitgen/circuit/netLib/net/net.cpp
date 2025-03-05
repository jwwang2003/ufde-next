#include "circuit/netLib/net/net.h"
#include "netlist.hpp"
#include "rtnetlist.hpp"

#include "main/arguments/Args.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

namespace BitGen {
namespace circuit {

//////////////////////////////////////////////////////////////////////////
// for XDL

void Net::constructFromXDL(const std::string &info) {
  _name = ((*_matchResults)[1]).str();
  string netInfo = ((*_matchResults)[2]).str();

  static const char *PPExp =
      // possibly leading whitespace:
      "\\s*"
      // pin or pip & its information: [1] = pin or pip, [2] = info, [3] = note
      "([outpin]+)\\s+(.*?)\\s*,|(#).*?\\n"; // after "#" is note, unused
  static boost::regex PPRegex(PPExp);

  boost::sregex_iterator it(netInfo.begin(), netInfo.end(), PPRegex);
  boost::sregex_iterator end;
  std::for_each(it, end, parsePinPip(this));

  delete _matchResults;

  if (args._device == "fdp1000k" || args._device == "fdp3000k") {
    _pips.adjustBiDirPips();
    _pips.adjustBRAMPips();
  }
}

bool parsePinPip::operator()(const cktBase::xdlMatch &what) const {
  string ppName = what[1].str();
  string ppInfo = what[2].str();
  string note = what[3].str();
  if (ppName == "inpin") {
    _curNet->addIPin(new pinNet(ppInfo, _curNet->getInstLib()));
  } else if (ppName == "outpin") {
    _curNet->addOPin(new pinNet(ppInfo, _curNet->getInstLib()));
  } else if (ppName == "pip") {
    _curNet->addPip(new pipNet(ppInfo));
  } else if (ppName == "cfg") {
  } // ignored
  else if (ppName == "" && note == "#") {
  } // ignored
  else
    throw CktException("pin/pip: invalid net info in xdl ... " + ppName);

  return true;
}

//////////////////////////////////////////////////////////////////////////
// for XML

void Net::constructFromXML() {
  COS::COSRTNet *net = static_cast<COS::COSRTNet *>(_nlBase);

  // pins
  // 		BOOST_FOREACH(const COS::Pin& pin, net->pins()){
  // 			pinNet* t = new pinNet(pin.name(), pin.owner().name(),
  // _refInstLib); 			pin.is_source() ? addOPin(t) :
  // addIPin(t);
  // 		}

  // pips
  for (const COS::PIP *pip : net->routing_pips()) {
    addPip(new pipNet(_archLibrary->get_inst_by_pos(pip->position())->name(),
                      pip->from(), pip->to(),
                      lexical_cast<string>(pip->dir())));
  }

  // *key* the followed two steps(adjustBiDirPips & adjustBRAMPips)
  //		 should be omitted when use our router, the reason should
  //       refer to STA's annotation(in HandleInterconnect function used
  //       "*зЂвт*") on my laptop
  //		_pips.adjustBiDirPips();
  //		_pips.adjustBRAMPips();
}
} // namespace circuit
} // namespace BitGen