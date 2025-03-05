#include "Args.h"
#include "log.h"

#include <boost/filesystem.hpp>
#include <fstream>

using namespace FDU;
using namespace boost::filesystem;

namespace po = boost::program_options;

//////////////////////////////////////////////////////////////////////////
// global
Args args;

//////////////////////////////////////////////////////////////////////////
// static data members

const char *Args::_ARCH_PATH = "\\arch\\";
const char *Args::_CIL_PATH = "\\cil\\";
const char *Args::_NETLIST_PATH = "\\netlist\\";
const char *Args::_BITFILE_PATH = "\\bitstream\\";
const char *Args::_LOGFILE_PATH = "\\log\\";
/*const char* Args::_FRMFILE_PATH = "\\outputs\\frm\\";*/
const char *Args::_FORMAT = "\t%1% = \"%2%\"\n";

//////////////////////////////////////////////////////////////////////////
// member functions

void Args::tryParse(int argc, char *argv[]) {
  try {
    po::options_description visible("Allowed options");
    visible.add_options()("arch,a", po::value<std::string>(),
                          "arch    file path (default at 1st)")(
        "cil,c", po::value<std::string>(),
        "cil     file path (default at 2nd)")(
        "netlist,n", po::value<std::string>(),
        "netlist file path (default at 3rd)")(
        "bitstream,b", po::value<std::string>(),
        "output  file path (default at 4th)")(
        "partialbitstream,p", po::value<int>(),
        "partial bitstream tilecol")("frm,f", "export  frm format bitstream")(
        "sopc,s", "use for sopc test : only valid for FDP80K")(
        "help,h", "show help messages");

    po::options_description hidden("Hidden options");
    hidden.add_options()("lib,l", po::value<std::string>(),
                         "library directory")(
        "work,w", po::value<std::string>(), "working directory")(
        "log,o", "output log file or not")("noencrypt,e",
                                           "run as NOT encrypt version");

    po::options_description cmdline;
    cmdline.add(visible).add(hidden);

    po::positional_options_description p;
    p.add("arch", 1).add("cil", 1).add("netlist", 1).add("bitstream", 1);

    po::variables_map vm;
    store(po::command_line_parser(argc, argv)
              .options(cmdline)
              .positional(p)
              .run(),
          vm);
    std::ifstream ifs("bitgen.conf");
    store(parse_config_file(ifs, cmdline), vm);
    notify(vm);

    if (vm.count("help")) {
      FDU_LOG(INFO).stream() << visible;
      dispHelp();
    }

    // 		if (vm.count("device")){
    // 			_device = vm["device"].as<string>();
    // 		}

    if (vm.count("arch")) {
      _arch = vm["arch"].as<std::string>();
    }

    if (vm.count("cil")) {
      _cil = vm["cil"].as<std::string>();
    }

    if (vm.count("netlist")) {
      _netlist = vm["netlist"].as<std::string>();
    }

    if (vm.count("bitstream")) {
      _bitstream = vm["bitstream"].as<std::string>();
    }

    _partialbitstream = -1;
    if (vm.count("partialbitstream")) {
      _partialbitstream = vm["partialbitstream"].as<int>();
    }

    if (vm.count("frm")) {
      _frmSwitch = true;
    } else {
      _frmSwitch = false;
    }

    if (vm.count("sopc")) {
      _sopcSwitch = true;
    } else {
      _sopcSwitch = false;
    }

    if (vm.count("lib")) {
      _lib_dir = vm["lib"].as<std::string>();
    }

    if (vm.count("work")) {
      _work_dir = vm["work"].as<std::string>();
    }

    if (vm.count("log")) {
      _logSwitch = true;
    } else {
      _logSwitch = false;
    }

    if (vm.count("noencrypt")) {
      _encrypt = false;
    } else {
      _encrypt = true;
    }

    if (vm.empty()) {
      dispHelp();
    }

    check();
  } catch (std::exception &e) {
    FDU_LOG(INFO) << e.what();
    dispHelp();
  }
}

void Args::dispHelp() const {
  FDU_LOG(INFO) << "\nUsage: ";
  FDU_LOG(INFO) << "  "
                << "bitgen [--arch|-a] <archFile.xml|XML> [--cil|-c] "
                   "<cilFile.xml|XML> [--netlist|-n] <netlist.xml|XML> "
                   "[--bitstream|-b] <bitFile> [--frm|-f] [--sopc|-s]";
  exit(EXIT_FAILURE);
}

void Args::check() {
  //	ASSERT(!_device.empty(),	exception("[CMD] Args: invalid chip
  // name"));
  ASSERT(!_arch.empty(), "[CMD] Args: invalid arch file path");
  ASSERT(!_cil.empty(), "[CMD] Args: invalid cil file path");
  ASSERT(!_netlist.empty(), "[CMD] Args: invalid netlist file path");
  ASSERT(!_bitstream.empty(), "[CMD] Args: invalid bitstream file path");

  // 	if(_bitstream.find_last_of('.') != string::npos)
  // 		cout << Warning("Args: bitstream file name has postfix") <<
  // endl;

  if (!_lib_dir.empty() && !_work_dir.empty()) {
    if (_logSwitch) {
      _log_dir =
          _work_dir + _LOGFILE_PATH + _netlist.substr(0, _netlist.find('.'));
      std::string default_log = _log_dir + "\\default";
      std::string circuit_log = _log_dir + "\\circuit";
      create_directory(path(_log_dir));
      create_directory(path(default_log));
      create_directory(path(circuit_log));
    }

    _arch = _lib_dir + _ARCH_PATH + _arch;
    _cil = _lib_dir + _CIL_PATH + _cil;
    _netlist = _work_dir + _NETLIST_PATH + _netlist;
    _bitstream = _work_dir + _BITFILE_PATH + _bitstream;
  }
}