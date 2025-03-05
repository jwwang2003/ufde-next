#ifndef _CIRCUIT_H_
#define _CIRCUIT_H_

#include "arch/archlib.hpp"
#include "circuit/design/design.h"
#include "circuit/instLib/instLib.h"
#include "circuit/netLib/netLib.h"
#include "utils/specialNets.h"

namespace BitGen {
namespace circuit {
class Circuit {
  COS::Design _design;
  instLib _insts;
  netLib _nets;
  // ARCH::FPGADesign*	_archLibrary;

public:
  explicit Circuit(const std::string &file, ARCH::FPGADesign *archLibrary = 0);

  void listInstCfgs(vecCfgs &cfgs) { _insts.listInstCfgs(cfgs); }
  void listNetPips(vecPips &pips) { _nets.listNetPips(pips); }
};

} // namespace circuit
} // namespace BitGen

#endif