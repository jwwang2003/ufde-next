#include "main/BGenApp/BGenApp.h"
#include "log.h"
#include "rtnetlist.hpp"
#include <iostream>

using namespace std;
using namespace FDU::CHIPTYPE;

namespace BitGen {

void BGenApp::tryBGen() {
  try {
    // LOG::init_log_file("log.txt","DEBUG");
    FDU_LOG(INFO) << Info("Load Chip Configuration Lib ... ");
    _cil = cilLibrary::loadCilLib(args._cil);
    FDU_LOG(INFO) << Info("done");
    FDU_LOG(INFO) << Info("finished 15%");

    FDU_LOG(INFO) << Info("Load Architecture Lib ... ");
    FPGADesign *archLibrary = FPGADesign::loadArchLib(args._arch);
    _cil->setArchLib(archLibrary);
    ASSERT(_cil->getChipName() == archLibrary->name(),
           "library: cil & arch not match");
    args._device = archLibrary->name();
    args._package = archLibrary->get_package();

    if (args._device != FDP80K && args._sopcSwitch == true) {
      throw std::runtime_error("BGenApp:\tsopc is only used for FDP80K...");
    }
    FDU_LOG(INFO) << Info("done");
    FDU_LOG(INFO) << Info("finished 30%");

    FDU_LOG(INFO) << Info("Load design ... ");
    if (args._device == FDP1000K || args._device == FDP3000K) {
      INIT_INNER_SCALE();
      INIT_SPECIAL_NETS();
      INIT_MAYBE_BIDIRECTIONAL_NETS();
    }
    _ckt = new Circuit(args._netlist, archLibrary);
    FDU_LOG(INFO) << Info("done");
    FDU_LOG(INFO) << Info("finished 60%");

    FDU_LOG(INFO) << Info("Construct & generate ... ");
    bstrGener generator(_ckt, _cil);
    generator.generate(args._bitstream, args._device, args._package);
    FDU_LOG(INFO) << Info("done");
    FDU_LOG(INFO) << Info("finished 100%");
    if (args._logSwitch) {
      generator.showLog(args._log_dir);
    }
  } catch (const std::runtime_error &e) {
    FDU_LOG(ERR) << "<Error> Fail to bitgen because ... ";
    FDU_LOG(ERR) << e.what();
    exit(EXIT_FAILURE);
  } catch (...) {
    FDU_LOG(ERR) << "<Error> Fail to bitgen because ... ";
    FDU_LOG(ERR) << "\t"
                 << "unknown exception occurred";
    exit(EXIT_FAILURE);
  }
}

} // namespace BitGen