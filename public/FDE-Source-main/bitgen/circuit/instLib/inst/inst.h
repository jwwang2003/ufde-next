#ifndef _INST_H_
#define _INST_H_

#include "arch/archlib.hpp"
#include "circuit/cktBase.h"
#include "circuit/instLib/inst/cfgInInst.h"
#include "circuit/instLib/inst/posInInst.h"

namespace BitGen {
namespace circuit {
using namespace ARCH;

class Inst : public cktBase {
private:
  std::string _instType;
  posInst _position;
  contCfgsInst _cfgs;
  FPGADesign *_archLibrary;

public:
  // for XML
  explicit Inst(FPGADesign *archLibrary = 0, COS::Object *instance = 0)
      : cktBase(instance), _archLibrary(archLibrary) {
    constructFromXML();
  }

  void adaptCfgTo(vecCfgs &cfgs, const cfgInst &cfg);
  void listInstCfgs(vecCfgs &cfgs);

  virtual void constructFromXML();
};

} // namespace circuit
} // namespace BitGen

#endif