#include "NLF_utils.hpp"
#include "utils/version.h"

namespace FDU {
namespace NLF {

using boost::format;

namespace UserInfo { // global variable, need extern if you wanna use it
format copy_rights_fmt("Release 1.0 - SliceRebuilder\n" FDE_COPYRIGHT "\n");
format progress_fmt("Progress  %1$3d%%: %2% ...\n");
format
    finish_fmt("Successfully finish Netlist Refinement. Elapsed Time: %1%s\n");
} // namespace UserInfo

} // namespace NLF
} // namespace FDU