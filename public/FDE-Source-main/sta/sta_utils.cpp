#include "sta_utils.hpp"
#include "utils/version.h"

namespace FDU {
namespace STA {

boost::format copy_rights_fmt("Release 1.3 - STA\n" FDE_COPYRIGHT "\n");
boost::format progress_fmt("Progress  %1$3d%%: %2% ...\n");
boost::format finish_fmt("Successfully finish RS. Elapsed Time: %1%s\n");

} // namespace STA
} // namespace FDU
