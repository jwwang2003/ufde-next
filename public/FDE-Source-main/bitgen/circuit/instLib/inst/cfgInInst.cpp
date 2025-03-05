#include "circuit/instLib/inst/cfgInInst.h"
#include "PropertyName.h"
#include "exception/exceptions.h"
#include "main/arguments/Args.h"

#include <boost/regex.hpp>

namespace BitGen {
namespace circuit {
using namespace FDU::CHIPTYPE;

void cfgInst::constructFromXML() {
  _instName = ""; // not used in xml netlist

  // old format: <config name="G#LUT" value="D=~A1"/>
  if (_name.find("#LUT") != string::npos) {
    _function._workMode = "equation";
    _name = _name[0];
  }

  // new format: <config name="G" value="#LUT:D=~A1"/>
  // omit #LUT:
  else if (_function._funcName.find("#LUT") != string::npos) {
    _function._workMode = "equation";
    _function._funcName = _function._funcName.substr(5);
  } else if (_name.find("#RAM") != string::npos) {
    _function._workMode = "srambit";
    _name = _name[0];
  } else if (_function._funcName.find("#RAM") != string::npos) {
    _function._workMode = "srambit";
    _function._funcName = _function._funcName.substr(5);
  } else
    _function._workMode = "naming";

  if (args._device == FDP1000K && _name == "F5MUX") {
    _name = "BXMUX";
    _function._funcName = "BX_B";
    _function._workMode = "naming";
  }

  // omit "D="
  if (_function._funcName.find_last_of("=") != string::npos)
    _function._funcName = _function._funcName.substr(2);

  _used = (_name == "_INST_PROP" || _function._funcName == "#OFF" ||
           _function._funcName.empty())
              ? false
              : true;
}

void contCfgsInst::addCfgXML(const std::string &name,
                             const std::string &funcName) {
  cfgInst *cfg = new cfgInst(name, funcName);
  if (cfg->isUsed())
    _cfgs.add(cfg);
  else
    delete cfg;
}

} // namespace circuit
} // namespace BitGen