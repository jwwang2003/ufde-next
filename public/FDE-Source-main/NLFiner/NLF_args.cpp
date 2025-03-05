#include "NLF_args.hpp"
#include "NLF_utils.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "log.h" // ASSERT

namespace FDU {
namespace NLF {

using namespace FDU;
using namespace std;
using namespace boost;
using namespace boost::program_options;

void NLFArgs::usage() const {
  cout << endl << "Usage: " << endl;
  cout << "\t"
       << "NLFiner "
          "[--xml|-d] <xmlfile> "
          "[--lib|-l] <libary> "
          "[--cfg|-c] <config> "
       << endl
       << endl;
  exit(EXIT_FAILURE);
}

void NLFArgs::check() {
  ASSERT(!_cfg.empty(), "invalid config lib file.");
  ASSERT(!_lib.empty(), "invalid cell lib file.");
  ASSERT(!_xml.empty(), "invalid netlist file.");
}

void NLFArgs::try_parse(int argc, char *argv[]) {
  try {
    options_description visible("Allowed options");
    visible.add_options()("xml,d", value<string>(),
                          "xml file (default at 1nd)")(
        "lib,l", value<string>(), "library file (default at 2th)")(
        "cfg,c", value<string>(), "config file (default at 3th)")(
        "output,o", value<string>(),
        "output file (default at 4th)")("help,h", "help messages");

    options_description hidden("Hidden options");
    hidden.add_options()("noencrypt,e", "run as non encrypted version");

    options_description cmd;
    cmd.add(visible).add(hidden);

    positional_options_description p;
    p.add("xml", 1).add("lib", 1).add("cfg", 1).add("output", 1);

    variables_map vm;
    store(command_line_parser(argc, argv).options(cmd).positional(p).run(), vm);
    ifstream conf("NLF.conf");
    store(parse_config_file(conf, cmd), vm);
    notify(vm);

    if (vm.count("help")) {
      cout << visible << endl;
      usage();
    }
    if (vm.count("xml")) {
      _xml = vm["xml"].as<string>();
    }
    if (vm.count("cfg")) {
      _cfg = vm["cfg"].as<string>();
    }
    if (vm.count("lib")) {
      _lib = vm["lib"].as<string>();
    }
    if (vm.count("output")) {
      _output = vm["output"].as<string>();
    }
    if (vm.count("noencrypt")) {
      _encrypt = false;
    } else {
      _encrypt = true;
    }
    if (vm.count("help") || vm.empty()) {
      usage();
    }
    check();
  } catch (const std::runtime_error &e) {
    cout << endl << e.what() << endl;
    usage();
  }
}
} // namespace NLF
} // namespace FDU