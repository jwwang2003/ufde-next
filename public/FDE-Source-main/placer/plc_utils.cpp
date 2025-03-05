#include "plc_utils.h"

namespace FDU {
namespace Place {

using namespace std;

namespace DEVICE {
/************************************************************************/
/* ��ʼ����ֵ�������3000k�ģ������v2�Ļ�����ᴦ��������Щֵ */
int NUM_SLICE_PER_TILE = 2;
int NUM_CARRY_PER_TILE = 2;
int NUM_LUT_INPUTS = 4;
vector<vector<int>> carry_chain(2);

/************************************************************************/
static const char *delay_table_types[] = {"clb2clb", "input2clb", "clb2output",
                                          "input2output",
                                          "num_delay_table_type"};
static EnumStringMap<DelayTableType> typemap(delay_table_types);
istream &operator>>(istream &s, DelayTableType &type) {
  type = typemap.readEnum(s);
  return s;
}
ostream &operator<<(ostream &s, DelayTableType type) {
  return typemap.writeEnum(s, type);
}
} // namespace DEVICE

/************************************************************************/
/* ����ִ�й����е������ʽ                                             */
/************************************************************************/
namespace CONSOLE {
// ERROR
format FILE_ERROR("[FILE_ERROR] %1%");
format PLC_ERROR("[PLC_ERROR] %1%");

// WARNING
format PLC_WARNING("[PLC_WARNING] %1%");

// APP INFO
format COPY_RIGHT("Release 1.0 - Place\n"
                  "Copyright (c) 2006-2009 FPGA-SoftwareGP.MEKeylab.FDU   All "
                  "rights reserved.\n");
format EFFORT_LEVEL("Effort Level  : %1%");
format PLC_MODE("Mode          : %1%");
format PROGRESS("Progress  %1$3d%%: %2% ...");
format FINISH("Successfully finish the placement. Elapsed Time: %1%s");
format DESIGN("Design        : \"%1%\", resource statistic:");
format RSC_IN_DESIGN("  * Amount of %1%: %2%");
format DEVICE_TYPE("Device        : \"%1%\", resource usage:");
format RSC_USAGE("  * Proportion of %1%: %2$.2f%%");
format RSC_SLICE("  * Proportion of %1%(LUT%2%): %3$.2f%%");

// SA Placer INFO
#ifdef VERBOSE
format INIT_COST("  * Initial t = %1%, cost = %2%");
format ITER_COST("  * Iteration %1%: t = %2%, rlim = %3%, cost = %4%");
format FINAL_COST("  * Final t = %1%, rlim = %2%, cost = %3%");
#else
format INIT_COST("  * Initial cost = %1%");
format ITER_COST("  * After %1% iterations cost = %2%");
format FINAL_COST("  * Final cost = %1%");
#endif
} // namespace CONSOLE

} // namespace Place
} // namespace FDU