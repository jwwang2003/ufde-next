#include "sta_app.hpp"
#include "sta_report.hpp"
#include "sta_utils.hpp"
#include <boost/timer/timer.hpp>
// #include "report.h"
// #include "reportmanager.hpp"
#include <chrono>

// #define NO_EXCEPTION_HANDLE

using namespace std;

namespace FDU {
namespace STA {

void STAApp::try_process(int argc, char *argv[]) {

#ifndef NO_EXCEPTION_HANDLE
  try {
#endif

     auto start = chrono::high_resolution_clock::now();

    arg_ = &STAArg::instance();
    design_ = new COS::TDesign;
    engine_ = new STAEngine(design_, arg_);

    //! 1. parse command
    if (!arg_->parse_command(argc, argv))
      return;
    //! 2. load files
    load_files();
    //! 3. run STA
    engine_->timing_analysis();
    //! 4. save files
    save_files();

    auto end = chrono::high_resolution_clock::now();

    FDU_LOG(INFO) << "success running STA in " << (chrono::duration_cast<chrono::milliseconds>(end - start).count()) << " ms";

#ifndef NO_EXCEPTION_HANDLE
  } catch (exception &e) {
    FDU_LOG(INFO) << "exception occured when performing STA:" << endl;
    FDU_LOG(INFO) << "\t" << e.what() << endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    FDU_LOG(INFO) << "unknown exception occured when performing STA" << endl;
    exit(EXIT_FAILURE);
  }
#endif
}

STAApp::~STAApp() {
  if (arg_)
    delete arg_;
  if (design_)
    delete design_;
  if (engine_)
    delete engine_;
}

void STAApp::load_files() {
  create_property<Point>(INSTANCE, "position");

  FDU_LOG(INFO) << (progress_fmt % "10" %
                    ("loading arch library \"" + arg_->arch + "\""))
                       .str();
  try {
    ARCH::FPGADesign::loadArchLib(arg_->arch, new RRGArchFactory());
  } catch (exception &e) {
    cerr << "cannot load arch library: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }

  FDU_LOG(INFO) << (progress_fmt % "20" %
                    ("loading netlist \"" + arg_->input + "\""))
                       .str();
  CosFactory::set_factory(new COSRTFactory());
  try {
    design_->load("xml", arg_->input);
  } catch (exception &e) {
    cerr << "cannot load netlist: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }

  FDU_LOG(INFO) << (progress_fmt % "30" %
                    ("loading interconnect library \"" + arg_->iclib + "\""))
                       .str();
  // ICLib::instance()->load_lib(arg_->iclib);
#ifndef NO_EXCEPTION_HANDLE
  try {
#endif
    ICLib::instance()->load_lib(arg_->iclib);
#ifndef NO_EXCEPTION_HANDLE
  } catch (exception &e) {
    cerr << "cannot load switch library: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }
#endif
}

void STAApp::save_files() {
  FDU_LOG(INFO) << (progress_fmt % "80" %
                    ("saving netlist \"" + arg_->output + "\""))
                       .str();
  try {
    design_->save("xml", arg_->output, arg_->encrypt_out);

    STAReport rpt(engine_);
    rpt.export_report(arg_->report, arg_->num_path);

    // StaReportManager* rpt = new StaReportManager("test", arg_);
    // rpt->writeReport("");
  } catch (exception &e) {
    cerr << "cannot save netlist: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }
}

} // namespace STA
} // namespace FDU
