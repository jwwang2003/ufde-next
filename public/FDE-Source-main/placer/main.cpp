#include "plc_app.h"

using namespace FDU::Place;
/************************************************************************/
/*	功能：判断是否可以交换，如果可以交换返回ture，否则返回false
 *	参数：void
 *	返回值：int
 *	说明：入口函数
 */
/************************************************************************/
int main(int argc, char *argv[]) {
  PlaceApp app;
  // 分析参数
  app.parse_command(argc, argv);
  // 开始布局
  app.try_process();

  return EXIT_SUCCESS;
}