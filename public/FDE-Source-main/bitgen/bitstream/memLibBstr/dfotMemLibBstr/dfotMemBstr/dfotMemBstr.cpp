#include "bitstream/memLibBstr/dfotMemLibBstr/dfotMemBstr/dfotMemBstr.h"

namespace BitGen {
namespace bitstream {

void dfotMemBstr::construct() {
  _name = "BRAM";
  _listSize = 128;
  _bitList.resize(128, "0000_0000");
}

} // namespace bitstream
} // namespace BitGen