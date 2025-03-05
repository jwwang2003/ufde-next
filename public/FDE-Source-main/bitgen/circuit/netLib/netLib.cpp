#include "circuit/netLib/netLib.h"

namespace BitGen {
namespace circuit {
using namespace boost;

void netLib::listNetPips(vecPips &pips) {
  for (Net *net : nets())
    net->listNetPips(pips);
}

} // namespace circuit
} // namespace BitGen