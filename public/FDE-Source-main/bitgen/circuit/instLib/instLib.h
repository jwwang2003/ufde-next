#ifndef _INSTLIB_H_
#define _INSTLIB_H_

#include "circuit/instLib/inst/inst.h"

namespace BitGen {
namespace circuit {

class instLib {
public:
  using instsType = cktContainer<Inst>::range_type;
  using const_instsType = cktContainer<Inst>::const_range_type;
  using instIter = cktContainer<Inst>::iterator;
  using const_instIter = cktContainer<Inst>::const_iterator;

private:
  cktContainer<Inst> _insts;

public:
  instsType insts() { return _insts.range(); }
  const_instsType insts() const { return _insts.range(); }

  Inst *addInst(Inst *inst) { return _insts.add(inst); }
  Inst &getInst(const std::string &instName) { return *insts().find(instName); }

  void listInstCfgs(vecCfgs &cfgs);
};

} // namespace circuit
} // namespace BitGen

#endif