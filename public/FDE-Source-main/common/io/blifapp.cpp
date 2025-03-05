#include "blifapp.h"

#include <boost/program_options.hpp>
#include <exception>
namespace po = boost::program_options;

#define USAGE                                                                  \
  "Usage: blif2xml -i blif_file -l library -x xml_file  [-v verilog_file] "    \
  "[-r report_file] [-h]"
#define print_usage(s)                                                         \
  {                                                                            \
    std::cout << (s) << std::endl << desc << std::endl;                        \
    return 1;                                                                  \
  }

ParseFor parse_for = ParseFor::MAPPER;

int BLIFApp::process_args(int argc, char *argv[]) {
  try {
    vector<string> s;
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "input,i", po::value<string>(),
        "input file to be parsed")("xml,x", po::value<string>(), "export xml")(
        "library,l", po::value<string>(),
        "cell library")("report,r", po::value<string>(), "report file")(
        "verilog,v", po::value<string>(), "verilog file")(
        "map,m", "parse for mapper")("pack,p", "parse for packer");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
      print_usage(USAGE);
    if (vm.count("input")) {
      _blif_file = vm["input"].as<string>();
    }
    if (vm.count("library"))
      _lib_file = vm["library"].as<string>();
    if (vm.count("report")) {
      _report_file = vm["report"].as<string>();
      _need_report = true;
    }
    if (vm.count("xml")) {
      _xml_file = vm["xml"].as<string>();
    }
    if (vm.count("verilog")) {
      _verilog_file = vm["verilog"].as<string>();
      _need_verilog = true;
    }
    if (vm.count("map")) {
      parse_for = ParseFor::MAPPER;
    }
    if (vm.count("pack")) {
      parse_for = ParseFor::PACKER;
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int BLIFApp::generate_xml() {
  try {
    // create design
    _design = new Design("blif design");

    // load cell_lib
    // std::cout << "Start loading cell libs...\n";
    _design->load(fileio::xml, _lib_file);
    // std::cout << "Load cell libs successfully!\n";

    // load blif file
    std::cout << "Start parsing blif file: " << _blif_file << "...\n";
    _design->load(fileio::blif, _blif_file, "work_lib");
    // std::cout << "Parse blif successfully!\n";

    /* Change design name */
    _design->rename(_design->top_cell().name());

    /* Remove empty net such as NIL */
    Net *net = _design->top_cell().nets().find("NIL");
    if (net)
      _design->top_cell().remove_net(*net);

    /* Check unused port */
    check_connectivity();

    // generate xml file
    // std::cout << "Start saving into xml file: " << _xml_file << "...\n";
    _design->save(fileio::xml, _xml_file);
    // std::cout << "File saved successfully!\n\n\n";
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}

int BLIFApp::generate_verilog() {
  try {
    _design->save(fileio::verilog, _verilog_file);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}

int BLIFApp::generate_report() { return 0; }

void BLIFApp::check_connectivity() {
  foreach (const Port &port, _design->top_cell().ports()) {
    const Pin &pin = port.cell_pin();
    const Net *net = pin.net();
    int source_num = net->source_pins().size();
    int sink_num = net->sink_pins().size();
    if (source_num && sink_num)
      ;
    else
      std::cout << "Warning : <Port> " << port.name() << " is dangling!\n";
  }
}