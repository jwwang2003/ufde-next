#include "sta_arg.hpp"
#include "log.h"
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

using namespace boost::program_options;
using namespace std;

namespace FDU {
namespace STA {

bool STAArg::parse_command(int argc, char *argv[]) {
  try {
    options_description visible("Allowed options");
    visible.add_options()("arch,a", value<string>(), "arch lib")(
        "input,i", value<string>(), "design input")("output,o", value<string>(),
                                                    "design output")(
        "iclib,l", value<string>(),
        "interconnect lib")("report,r", value<string>(), "report file")(
        "config,c", value<string>(), "config file default = \"STA.conf\"")(
        "grmname,g", value<string>(), "grm name default = \"GSB\"")(
        "num_path,n", value<int>(), "number of paths to export, default = 5")(
        "spice,s", "dump spice netlist for each net")(
        "dot,d", "dump dot file for each net")("help,h", "help");

    options_description hidden("Hidden options");
    hidden.add_options()("noencrypt,e", "whether to use nonencrypt output");

    options_description cmd;
    cmd.add(visible).add(hidden);

    variables_map vm;
    store(command_line_parser(argc, argv).options(cmd).run(), vm);
    notify(vm);

    if (vm.count("config")) {
      this->config = vm["config"].as<string>();
    }

    ifstream config_file(this->config.c_str());
    if (config_file) {
      store(parse_config_file(config_file, cmd), vm);
    } else {
      FDU_LOG(WARN) << "cannot open config file: " << this->config;
    }

    notify(vm);

    if (vm.count("help")) {
      cout << visible << endl;
      return false;
    }

    if (vm.count("arch")) {
      this->arch = vm["arch"].as<string>();
    }

    if (vm.count("input")) {
      this->input = vm["input"].as<string>();
    }

    if (vm.count("output")) {
      this->output = vm["output"].as<string>();
    }

    if (vm.count("iclib")) {
      this->iclib = vm["iclib"].as<string>();
    }

    if (vm.count("report")) {
      this->report = vm["report"].as<string>();
    }

    if (vm.count("num_path")) {
      this->num_path = vm["num_path"].as<int>();
    }

    if (vm.count("noencrypt")) {
      this->encrypt_out = false;
    }

    if (vm.count("spice")) {
      this->dump_spice_for_net = true;
    }

    if (vm.count("dot")) {
      this->dump_dot_for_net = true;
    }
    //--------------------------------------------------------
    // check integrity

    ASSERT(!arch.empty(), "arch file name empty");
    ASSERT(!input.empty(), "design input file name empty");
    ASSERT(!iclib.empty(), "iclib file name empty");

    if (output.empty()) {
      string::size_type dot_pos = input.rfind('.');
      if (dot_pos != string::npos)
        output = input.substr(0, dot_pos) + "_out.xml";
      else
        output = input + "_out.xml";
    }

    if (report.empty()) {
      string::size_type dot_pos = input.rfind('.');
      if (dot_pos != string::npos)
        report = input.substr(0, dot_pos) + ".rpt";
      else
        report = input + ".rpt";
    }

  } catch (exception &e) {
    cout << "exception occured when parsing command:" << endl;
    cout << "\t" << e.what() << endl;
    cout << "try \"--help\" to see usage\n" << endl;
    return false;
  } catch (...) {
    cout << "unknown exception occured when parsing command" << endl;
    cout << "try \"--help\" to see usage\n" << endl;
    return false;
  }

  return true;
}

} // namespace STA
} // namespace FDU
