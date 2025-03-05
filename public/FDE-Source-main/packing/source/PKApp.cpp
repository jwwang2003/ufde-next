#include "PKApp.h"
#include "io/usingsimvl.h"
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ctime>
#include <iostream>

namespace PACK {

namespace po = boost::program_options;
using namespace std;

PKApp PKApp::instance_;
string CHIP_TYPE_NAME::chip_type_ = "undefine";

void PKApp::try_process() {
  time_t t_start, t_end;
  time(&t_start);
  // srand(t_start);

  initialize();
  packer_.try_pack(&design_, fEncry_);

  if (!sim_fname_.empty())
    design_.save("pack_sim_verilog", sim_fname_);
  if (!result_fname_.empty())
    design_.save("xml", result_fname_, fEncry_);

  time(&t_end);
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_PACK_FINISH) %
                       difftime(t_end, t_start);
}

void PKApp::initialize() {
  COS::IO::using_simvl();
  // packer_.block_config();//call the function in PKConfig.h in the
  // end//modified by hz
  if (!log_fname_.empty())
    FDU::LOG::init_log_file(log_fname_);
  packer_.load_config_lib(config_fname_); // added by hz
  FDU_LOG(INFO) << "Loading config file ... " << config_fname_;

  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_LOAD_CLIB) % lib_fname_;
  try {
    CosFactory::set_factory(
        new PKFactory()); /*sophie:setFactory before load celllib, because you
                             may operate on lib contents*/
    design_.load("xml", lib_fname_);
  } catch (std::ios_base::failure &e) {
    FDU_LOG(ERR) << "cannot open library file: " << lib_fname_
                 << ", please check the file name and path.\n";
    exit(EXIT_FAILURE);
  }

  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_LOAD_NL) % netlist_fname_;
  try {
    if (netlist_fname_.find(".xml") != string::npos) {
      design_.load("xml", netlist_fname_);
    } else if (netlist_fname_.find(".blif") != string::npos) {
      design_.load("blif", netlist_fname_);
    } else {
      FDU_LOG(ERR) << "Can not recogonize this type of file: "
                   << netlist_fname_;
      exit(EXIT_FAILURE);
    }
    // design_.load("edif", netlist_fname_);
  } catch (std::ios_base::failure &e) {
    FDU_LOG(ERR) << "cannot open netlist file: " << netlist_fname_
                 << ", please check the file name and path.\n";
    exit(EXIT_FAILURE);
  }

  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_LOAD_PLIB) % rule_fname_;
  try {
    packer_.load_rule_lib(rule_fname_, &design_);
  } catch (std::ios_base::failure &e) {
    FDU_LOG(ERR) << "cannot open rule library file: " << rule_fname_
                 << ", please check the file name and path.\n";
    exit(EXIT_FAILURE);
  }
}

void PKApp::parse_command(int argc, char *argv[]) {
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_CPY_RGHT);
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_PARSE_CMD);
  try {
    desc_.add_options()("output,o", po::value<string>(), "Output file")(
        "chiptype,c", po::value<string>(),
        "Select chip type")("fde,f", po::value<string>(), "Select fde file")(
        "sim,s", po::value<string>(), "Output verilog file for simulation")(
        "xdl,x", po::value<string>(), "Output xdl file for ise verificaton")(
        "report,p", po::value<string>(), "Output a simple report")(
        "log,m", po::value<string>(), "Log file") // hz
        ("noencrypt,e", "run as NOT encrypt version")("help,h",
                                                      "Produce help message");

    po::options_description hidden("Hidden options");
    hidden.add_options()("netlist,n", po::value<string>(), "Netlist File")(
        "lib,l", po::value<string>(),
        "Library File")("rule,r", po::value<string>(), "RuleLibrary File")(
        "config,g", po::value<string>(), "Configlibrary File"); // hz

    po::options_description cmdline_options;
    cmdline_options.add(desc_).add(hidden);

    po::positional_options_description p;
    p.add("netlist", 1);
    p.add("lib", 1);
    p.add("rule", 1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
                  .options(cmdline_options)
                  .positional(p)
                  .run(),
              vm);
    po::notify(vm);

    if (vm.count("output"))
      result_fname_ = vm["output"].as<string>();
    if (vm.count("chiptype"))
      chip_type_ = vm["chiptype"].as<string>();
    if (vm.count("netlist"))
      netlist_fname_ = vm["netlist"].as<string>();

    // 添加对fde文件的支持
    if (vm.count("fde")) {
      string _fde_file = vm["fde"].as<string>();
      boost::property_tree::ptree prj;
      read_info(_fde_file, prj);
      //_prjName = prj.get<string>("project.name");
      lib_fname_ = prj.get<string>("library.fdpblk");
      rule_fname_ = prj.get<string>("library.plib");
      FDU_LOG(INFO) << "Progress    3%%: loading fde file '" << _fde_file
                    << "'...";
    } else {
      /*cerr << "[Mapping Error]: Command Line Error: no fde file" << endl;
      cerr << desc << endl;
      exit(-1);*/
    }
    if (vm.count("lib"))
      lib_fname_ = vm["lib"].as<string>();
    if (vm.count("rule"))
      rule_fname_ = vm["rule"].as<string>();
    if (vm.count("config")) // hz
      config_fname_ = vm["config"].as<string>();
    if (vm.count("log")) // hz
      log_fname_ = vm["log"].as<string>();
    if (vm.count("sim"))
      sim_fname_ = vm["sim"].as<string>();
    if (vm.count("xdl"))
      xdl_fname_ = vm["xdl"].as<string>();
    if (vm.count("report"))
      rpt_fname_ = vm["report"].as<string>();
    if (vm.count("noencrypt"))
      fEncry_ = false;

    if (vm.count("help"))
      display_usage();

    cmd_error_check();
  } catch (exception &e) {
    cout << e.what() << endl;
    display_usage();
  }
}

void PKApp::cmd_error_check() {}

void PKApp::display_usage() {
  cout << "\nUsage: " << endl;
  cout << " pack <netFile.xml|XML> <libFile.xml|XML> <ruleFile.xml|XML> "
          "[-o PackResultFile] [-s SimFile] [PACK options]";
  cout << "\n\n";
  cout << desc_ << endl;
  exit(EXIT_FAILURE);
}
void PKApp::design_summary() {
  cout << "\nDesign Summary\n" << endl;
  cout << "--------------";
  cout << "\nLogic Utilization:";
  cout << "\n\t\tNumber of Slice Flip Flops:";
  cout << "\n\t\tNumber of 4 input LUTs:";
  cout << "\nTotal Number of 4 input LUTs: ";
  cout << "\n\t\tNumber of bonded IOBs: ";
  cout << "\n\t\tNumber of GCLKs: ";
  cout << "\n\t\t Number of GCLKIOBs: ";
}
} // namespace PACK