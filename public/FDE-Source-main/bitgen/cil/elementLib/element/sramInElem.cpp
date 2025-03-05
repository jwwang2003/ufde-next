#include "cil/elementLib/element/sramInElem.h"

#include <boost/lexical_cast.hpp>

namespace FDU {
namespace cil_lib {
using namespace boost;

void sramElem::saveTo(vecBits &bits) {
  bitTile tBit;
  tBit._sramName = _name;
  tBit._bitContent = _content;
  bits.push_back(tBit);
}

void contSramsElem::listSrams(vecBits &bits) {
  for (sramElem *sram : srams())
    sram->saveTo(bits);
}

} // namespace cil_lib
} // namespace FDU