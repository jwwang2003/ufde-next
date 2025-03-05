#include "RTApp.h"
#include <ctime>
#include <iostream>

int main(int argc, char *argv[]) {
  FDU::RT::RTApp rt_app; // 在RTApp.h中有定义

  rt_app.parse_command(argc, argv); // 读取命令行参数
  rt_app.try_process();
  return 0;
}