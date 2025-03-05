#include "NLF_app.hpp"
#include "NLF_utils.hpp"
#include "NLFiner.hpp"
#include "log.h"

#include <boost/format.hpp>
#include <iostream>
#include <boost/timer/timer.hpp>
#include <chrono>

namespace FDU {
namespace NLF {

using namespace COS;
using namespace std;
using namespace UserInfo;

void NLFApp::parse_command(int argc, char *argv[]) {
  FDU_LOG(INFO) << copy_rights_fmt.str() << endl;
  _args.try_parse(argc, argv);
}

void NLFApp::try_process() {
  auto start = chrono::high_resolution_clock::now();

#ifdef EXCEPTION_HANDLE
  try {
#endif
    load_files();
    refine_netlist();
    save_files();

    auto end = chrono::high_resolution_clock::now();

    FDU_LOG(INFO) << (finish_fmt % chrono::duration_cast<chrono::milliseconds>(end - start).count()).str();

#ifdef EXCEPTION_HANDLE
  } catch (exception &e) {
    FDU_LOG(ERR) << e.what() << endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    FDU_LOG(ERR) << "unknown error occurred." << endl;
    exit(EXIT_FAILURE);
  }
#endif
}

void NLFApp::load_files() {
  FDU_LOG(INFO) << (progress_fmt % 10 % "Loading files").str();
  _design = new COS::Design;
  _cell_lib = new COS::Design;
  CosFactory::set_factory(new COSRTFactory()); // load timing & pip info
  _design->load("xml", _args._xml);
  FDU_LOG(INFO) << "complete load design " << _args._xml;
  _cell_lib->load("xml", _args._lib);
  FDU_LOG(INFO) << "complete load cell lib " << _args._lib;
  _repo = new ConfigRepo;
  ConfigLoader conf_loader(_repo);
  conf_loader.load(_args._cfg);
  FDU_LOG(INFO) << "complete load config lib " << _args._cfg;
}

void NLFApp::save_files() {
  FDU_LOG(INFO) << (progress_fmt % 90 % "Saving files").str();
  _design->save("xml", _args._output, _args._encrypt);
  FDU_LOG(INFO) << "complete save design file " << _args._output;
}

void NLFApp::refine_netlist() {
  FDU_LOG(INFO) << (progress_fmt % 50 % "Begin refining netlist").str();
  NLFiner refiner(_design, _cell_lib, _repo);
  refiner.refine();
}

} // namespace NLF
} // namespace FDU
