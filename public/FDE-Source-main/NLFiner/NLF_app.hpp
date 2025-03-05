#ifndef __NLF_APP_H__
#define __NLF_APP_H__

#include "NLF_args.hpp"
#include "config.hpp"
#include "log.h"
#include "netlist/netlist.hpp"

namespace FDU {
namespace NLF {

class NLFApp {
public:
  NLFApp() : _design(0), _repo(0) {
    FDU::LOG::init_log_file("run.log", "NLFiner");
  }
  ~NLFApp() {
    if (_design)
      delete _design;
    if (_cell_lib)
      delete _cell_lib;
    if (_repo)
      delete _repo;
  }

  void parse_command(int argc, char *argv[]);
  void try_process();

private:
  void load_files();
  void save_files();
  void refine_netlist();

  NLFArgs _args;
  COS::Design *_design;
  COS::Design *_cell_lib;
  ConfigRepo *_repo;
};
} // namespace NLF
} // namespace FDU

#endif