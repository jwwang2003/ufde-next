
#include "plc_args.h"
#include "log.h"
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>

namespace FDU {
namespace Place {

using namespace std;
using namespace boost;
using namespace boost::program_options;

void PlaceArgs::disp_help() const {
  cout << "\nUsage: " << endl;
  cout << "  "
       << "placer [--arch|-a] <arch_lib.xml> "
          "[--input|-i] <input_netlist.xml> "
          "[--output|-o] <output_netlist.xml> "
          "[--delay|-d] [delay_lib.xml] "
          "[--cst|-c] [constraint.xml] "
          "[--jtag|-j] [jtag_csts.txt] "
          "[--effort_level|-l] [standard | medium | high]"
          "[--bounding_box|-b] "
          "[--timing_driven|-t] "
          "[--update_rand_seed|-u] ";
  cout << "\n\n";
  exit(EXIT_FAILURE);
}

void PlaceArgs::check() const {
  ASSERT(!_arch_lib.empty(), CONSOLE::PLC_ERROR % "invalid arch file.");
  ASSERT(!_input_nl.empty(), CONSOLE::PLC_ERROR % "invalid input netlist.");
  ASSERT(!_output_nl.empty(), CONSOLE::PLC_ERROR % "invalid netlist file.");
}

void PlaceArgs::try_parse(int argc, char *argv[]) {
  try {
    options_description visible("Allowed options");
    visible.add_options()("arch,a", value<string>(),
                          "arch   library(default at 1st)")(
        "library,f", value<string>(), "template cell library file")(
        "input,i", value<string>(), "input  netlist(default at 2nd)")(
        "output,o", value<string>(), "output netlist(default at 3rd)")(
        "delay,d", value<string>(),
        "delay	 library")("cst,c", value<string>(), "place constraints")(
        "jtag,j", value<string>(), "jtag  constraints (export)")(
        "effort_level,l", value<string>(),
        "place effort level")("bounding_box,b", "bounding_box placer")(
        "timing_driven,t", "timing_driven placer")("update_rand_seed,u",
                                                   "update seed of randomizer")(
        "fde,f", value<string>(), "set fde file")("help,h", "help messages");

    options_description hidden("Hidden options");
    hidden.add_options()("noencrypt,e", "run as NOT encrypt version");

    options_description cmd;
    cmd.add(visible).add(hidden);

    positional_options_description p;
    p.add("arch", 1).add("input", 1).add("output", 1);

    variables_map vm;
    store(command_line_parser(argc, argv).options(cmd).positional(p).run(), vm);
    ifstream conf("place.conf");
    store(parse_config_file(conf, cmd), vm);
    notify(vm);

    if (vm.count("help")) {
      cout << visible << endl;
      disp_help();
    }

    if (vm.count("fde")) {
      string _fde_file = vm["fde"].as<string>();
      boost::property_tree::ptree prj;
      read_info(_fde_file, prj);
      _arch_lib = prj.get<string>("library.arch");
    }

    if (vm.count("arch")) {
      _arch_lib = vm["arch"].as<string>();
    }

    if (vm.count("library")) {
      _library = vm["library"].as<string>();
    }

    if (vm.count("input")) {
      _input_nl = vm["input"].as<string>();
    }

    if (vm.count("output")) {
      _output_nl = vm["output"].as<string>();
    }

    if (vm.count("delay")) {
      _delay_lib = vm["delay"].as<string>();
    }

    if (vm.count("cst")) {
      _plc_csts = vm["cst"].as<string>();
    }

    if (vm.count("jtag")) {
      _jtag_csts = vm["jtag"].as<string>();
    } else {
      _jtag_csts.clear();
    }

    if (vm.count("bounding_box") && vm.count("timing_driven")) {
      throw std::runtime_error(
          "ONLY allow to use either bounding_box or timing_driven placer");
    }

    if (vm.count("effort_level")) {
      string level = vm["effort_level"].as<string>();
      _effort_level = level == "medium" ? PLACER::MEDIUM
                      : level == "high" ? PLACER::HIGH
                                        : PLACER::STANDARD;
    } else {
      _effort_level = PLACER::STANDARD;
    }

    if (vm.count("timing_driven")) {
      _mode = PLACER::TIMING_DRIVEN;
    } else if (vm.count("bounding_box")) {
      _mode = PLACER::BOUNDING_BOX;
    } else {
      _mode = PLACER::BOUNDING_BOX;
    }

    if (vm.count("update_rand_seed")) {
      _update_rand_seed = true;
    } else {
      _update_rand_seed = false;
    }

    if (vm.count("noencrypt")) {
      _encrypt = false;
    } else {
      _encrypt = true;
    }

    if (vm.empty())
      disp_help();

    check();
  } catch (const std::runtime_error &e) {
    FDU_LOG(ERR) << endl << e.what() << endl;
    disp_help();
  }
}

} // namespace Place
} // namespace FDU