#include "bitstream/memLibBstr/dfotMemLibBstr/dfotMemLibBstr.h"

namespace BitGen {
namespace bitstream {

void dfotMemLibBstr::construct() { addDfotMem(new dfotMemBstr()); }

} // namespace bitstream
} // namespace BitGen