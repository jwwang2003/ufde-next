#include "RTApp.h"
#include <ctime>
#include <iostream>

int main(int argc, char *argv[]) {
  FDU::RT::RTApp rt_app; // ��RTApp.h���ж���

  rt_app.parse_command(argc, argv); // ��ȡ�����в���
  rt_app.try_process();
  return 0;
}