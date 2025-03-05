#ifndef PLCAPP_H
#define PLCAPP_H

#include "log.h"
#include "plc_algorithm.h"
#include "plc_args.h"

namespace FDU {
namespace Place {

using COS::TDesign;

class PlaceApp {
public:
  PlaceApp() : _placer(&_design) {
    LOG::init_log_file("place.log", "place");
  } // ��ʼ��log�ļ�

  void parse_command(int argc, char *argv[]);
  void try_process();

protected:
  void load_files();
  void export_jtag_csts();

private:
  PlaceArgs _args; // ���ֵĲ���

  TDesign _design; // �û�����
  SAPlacer
      _placer; // ��������ģ���˻�����㷨��������ɣ�����cost����FloorPlan�������
};

} // namespace Place
} // namespace FDU

#endif