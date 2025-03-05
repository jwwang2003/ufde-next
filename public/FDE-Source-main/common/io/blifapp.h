#ifndef BLIF_APP_HPP
#define BLIF_APP_HPP

#include "netlist.hpp"
#include <string>
#include <vector>

using namespace FudanFPGA::netlist;

enum ParseFor { MAPPER, PACKER };

class BLIFApp {
public:
  BLIFApp() : _need_report(false), _need_verilog(false) {}

  ~BLIFApp() { delete _design; }
  int process_args(int argc, char *argv[]);
  int generate_xml();
  int generate_verilog();
  int generate_report();
  void check_connectivity();

private:
  string _blif_file, _lib_file, _report_file, _xml_file, _verilog_file;
  bool _need_report, _need_verilog;
  Design *_design;
};

#endif
