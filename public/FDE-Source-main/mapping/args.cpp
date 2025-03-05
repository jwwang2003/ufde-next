#include "args.hpp"
#include "log.h"
#include "version.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

Args::Args(int argc, char **argv) {
  try {
    po::options_description desc("Allowed options");
    desc.add_options()("input,i", po::value<string>(), "set input file name")(
        "output,o", po::value<string>(), "set mapping output file")(
        "celllib,c", po::value<string>(), "set cell library")(
        "verilog,v", po::value<string>(), "set output verilog file")(
        "lut,k", po::value<int>(), "set mapped lut-size")("yosys,y",
                                                          "set yosys flow")(
        "noencrypt,e", "run as NOT encrypt version")("help,h",
                                                     "produce help message");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // cout << fde_copyright("Mapping");
    // FDU_LOG(INFO) << Info("Successfullly map the logical netlist!");

    if (vm.count("help")) {
      cout << desc << endl;
      exit(EXIT_FAILURE);
    }
    if (vm.count("celllib")) {
      cellLib = vm["celllib"].as<string>();
      FDU_LOG(INFO) << "Cell library:  " << cellLib;
    }
    if (vm.count("input")) {
      static const pair<string, string> fileTypes[] = {
          {".xml", "xml"},
          //{".blif", "blif"},
          {".edf", "edif"},
          {".edif", "edif"},
      };
      inputFile = vm["input"].as<string>();
      for (auto [ext, type] : fileTypes) {
        auto pos = inputFile.rfind(ext);
        if (pos == inputFile.size() - ext.size()) {
          inputType = type;
          prjName = inputFile.substr(0, pos) + "_map";
          break;
        }
      }
      ASSERT(!inputType.empty(), "invalid input type");
    }
    FDU_LOG(INFO) << "Read from:     " << inputFile;

    if (vm.count("output")) {
      outputFile = vm["output"].as<string>();
      auto pos = outputFile.rfind(".xml");
      if (pos == outputFile.size() - 4)
        prjName = outputFile.substr(0, pos);
    } else {
      outputFile = prjName + ".xml";
    }
    FDU_LOG(INFO) << "Write to:    " << outputFile;

    if (vm.count("verilog")) {
      verilogFile = vm["verilog"].as<string>();
      FDU_LOG(INFO) << "Write verilog:  " << verilogFile;
    }

    if (vm.count("lut")) {
      lutSize = vm["lut"].as<int>();
      ASSERT(lutSize >= 2 && lutSize <= 6, "LUT size must between 2-6");
    }

    if (vm.count("yosys")) {
      flow = Yosys;
    }
    if (vm.count("noencrypt")) {
      fEncry = false;
    }
  } catch (po::error &e) {
    ASSERT(0, e.what());
  }
}
