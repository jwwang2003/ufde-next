#include "circuit/instLib/instLib.h"

namespace BitGen {
namespace circuit {
using namespace boost;

void instLib::listInstCfgs(vecCfgs &cfgs) {
  for (Inst *inst : insts())
    inst->listInstCfgs(cfgs);
}

} // namespace circuit
} // namespace BitGen