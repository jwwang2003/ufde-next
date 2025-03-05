#include "RTCstLoadHandler.h"
#include "RTMsg.h"
#include "RTNode.h"
#include "Router.h"
#include "arch/archlib.hpp"
#include "io/usingrtsimvl.h"
#include "property.hpp"
#include "rutils.h"
#include <exception>

namespace FDU {
namespace RT {
namespace po = boost::program_options;
using namespace COS;
using namespace std;
using namespace ARCH;

Property<string> &cil_filename =
    create_property<string>(COS::DESIGN, "cil_fname");

RTApp::RTApp()
    : desc_("Options"), encrypted_(true), router_(&Router::Instance()) {
} // ����һ��routerʵ���ĵ�ַ

void RTApp::parse_command(int argc, char *argv[]) {
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_CPY_RGHT);
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_PAR_CMD); // �����־��Ϣ
#ifndef _DEBUG
  try {
#endif
    po::variables_map vm;
    store_cmd(argc, argv, vm); // �洢���������뵽vm������
    parse_vm(vm);              // ʶ�����������ִ����Ӧ����
    error_check();             // ������������
#ifndef _DEBUG
  } catch (exception &e) {
    FDU_LOG(ERR) << e.what() << endl;
    display_usage_and_exit();
  }
#endif
}

void RTApp::store_cmd(int argc, char *argv[],
                      po::variables_map &vm) // �洢�����в���
{
  po::options_description required_desc("Required options"); // ��Ҫѡ��
  required_desc.add_options()("netlist,n", po::value<string>(), "Netlist File")(
      "arch,a", po::value<string>(), "Arch File");

  po::options_description optional_desc("Optional options"); // ��ѡѡ��
  optional_desc.add_options()("output,o", po::value<string>(), "Output file")(
      "breadth-first,b", "Breadth-first routing")("timing-driven,t",
                                                  "Timing-driven routing")(
      "directed-search,d", "Directed-search routing(Default)")(
      "noencrypt,e", "run as NOT encrypt version")("help,h",
                                                   "Produce help message")(
      "astar-factor,s", po::value<double>(),
      "Astar factor")("cst,c", po::value<string>(), "router constraint file")(
      "cil,i", po::value<string>(), "Cillib file")(
      "verilog,v", po::value<string>(), "Output verilog file for simulation");

  desc_.add(required_desc).add(optional_desc);

  po::positional_options_description p; // ��ʡȥnetlist�Ĳ���ѡ��
  p.add("netlist", 1);
  p.add("arch", 1);
  po::store(
      po::command_line_parser(argc, argv).options(desc_).positional(p).run(),
      vm);
  po::notify(vm);
}

void RTApp::parse_vm(po::variables_map &vm) // ʶ���������
{
  if (vm.count("help") || vm.size() == 0)
    display_usage_and_exit(); // ��ӡ����˳�
  if (vm.count("output"))
    output_fname_ = vm["output"].as<string>();
  if (vm.count("netlist"))
    netlist_fname_ = vm["netlist"].as<string>();
  if (vm.count("arch"))
    arch_fname_ = vm["arch"].as<string>();
  if (vm.count("cst"))
    cst_fname_ = vm["cst"].as<string>(); // constraint file
  if (vm.count("verilog"))
    sim_fname_ = vm["verilog"].as<string>(); // add_by_sxb
  if (vm.count("cil"))
    cil_fname_ = vm["cil"].as<string>(); // add_by_sxb

  set_algorithm(vm);

  if (vm.count("astar-factor")) {
    rt_params_.astar_fac = vm["astar-factor"].as<double>();
  }
  if (vm.count("noencrypt"))
    encrypted_ = false;
}

void RTApp::set_algorithm(
    po::variables_map &vm) // ����route���㷨��Ĭ��directed-search��
{
  if (vm.count("timing-driven")) {
    rt_params_.rt_algo = TIMING_DRIVEN;
    rt_params_.bcost_type = DELAY_NORMALIZED;
    rt_params_.astar_fac = 0;
  }
  if (vm.count("breadth-first")) {
    rt_params_.rt_algo = BREADTH_FIRST;
    rt_params_.astar_fac = 0;
  }
  if (vm.count("directed-search")) {
    rt_params_.rt_algo = DIRECTED_SEARCH;
  }
}

void RTApp::
    set_default_output_file_name() // ��û���趨����ļ����趨Ĭ������ļ�(_rt.xml)
{
  if (!file_extension_is_xml(netlist_fname_)) {
    FDU_LOG(ERR) << ErrMsg(ErrMsg::CMDERR_ILL_FILE_FMT) %
                        "netlist"; // ��netlist����xml�������Ч
    display_usage_and_exit();
  } else {
    string::size_type fpos = netlist_fname_.find_last_of('_');
    if (fpos != string::npos)
      output_fname_ = netlist_fname_.substr(0, fpos) + "_rt.xml";
    else
      output_fname_ = netlist_fname_ + "_rt.xml";
  }
}

bool RTApp::file_extension_is_xml(
    const std::string &file_name) const // �ж��ļ���׺�Ƿ�Ϊ .XML
{
  string file_ext_name;
  string::size_type fpos = file_name.find_last_of('.');
  if (fpos != string::npos)
    file_ext_name = file_name.substr(fpos, file_name.size() - fpos);
  return (file_ext_name == ".xml" || file_ext_name == ".XML");
}

void RTApp::error_check() // ��������ж�
{
  if (netlist_fname_ == "") {
    FDU_LOG(ERR) << ErrMsg(ErrMsg::CMDERR_FILE_MISS) % "netlist";
    display_usage_and_exit();
  }
  if (arch_fname_ == "") {
    FDU_LOG(ERR) << ErrMsg(ErrMsg::CMDERR_FILE_MISS) % "arch";
    display_usage_and_exit();
  }
  if (output_fname_ == "") {
    set_default_output_file_name();
  } else {
    if (!file_extension_is_xml(output_fname_))
      output_fname_ =
          output_fname_ + ".xml"; // �����ļ�û��.xml��׺�����xml��׺
  }

  if (rt_params_.astar_fac != 0 && rt_params_.rt_algo != DIRECTED_SEARCH) {
    FDU_LOG(WARN) << WarMsg(WarMsg::CMDWNG_AMBIGUITY_ALGORITHM);
    rt_params_.rt_algo = DIRECTED_SEARCH;
  }
}

void RTApp::display_usage_and_exit() const // ���ʹ����Ϣ���˳�
{
  FDU_LOG(INFO) << "\nUsage: ";
  FDU_LOG(INFO) << "\trouter <netFile.xml|XML> <archFile.xml|XML> [-o "
                   "RouteResultFile.xml]";
  FDU_LOG(INFO) << "\n";
  FDU_LOG(INFO).stream() << desc_;
  exit(EXIT_FAILURE);
}

void RTApp::try_process() // route����
{
#ifndef _DEBUG
  try {
#endif
    time_t t_begin, t_end;
    time(&t_begin); // ��ȡ��ʼrouteʱ��

    load_files(); // ��ȡ����ļ�

    if (router_->try_route(&design_, rt_params_)) {

      FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_EXPT_RST) % output_fname_;
      design_.save("xml", output_fname_, encrypted_);

      if (!sim_fname_.empty()) {
        COS::IO::using_rtsimvl();
        design_.save("route_sim_verilog", sim_fname_);
        FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_SIM_FILE) % sim_fname_;
      }

      time(&t_end);
      FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_FNL_RT) % difftime(t_end, t_begin);
    } else {
      FDU_LOG(ERR) << ErrMsg(ErrMsg::RTERR_ST_RT);
      exit(EXIT_FAILURE);
    }

#ifndef _DEBUG
  } catch (exception &e) {
    FDU_LOG(ERR) << e.what();
    FDU_LOG(ERR) << ErrMsg(ErrMsg::RTERR_ST_RT);
    exit(EXIT_FAILURE);
  }
#endif
}

void RTApp::load_files() // ��ȡ����ļ�
{
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_LOAD_NL) %
                       netlist_fname_; // netlist �ļ�
  CosFactory::set_factory(new RTFactory());
  create_property<Point>(COS::INSTANCE, RT_CONST::POSITION);
  design_.load("xml", netlist_fname_);

  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_LOAD_ARCH) % arch_fname_; // arch�ļ�
  ARCH::FPGADesign::loadArchLib(arch_fname_, new RRGArchFactory());

  if (!cst_fname_.empty()) { // cst�ļ�
    RTCstLoadHandler cstLoadHandler(cst_fname_, router_->getCstNets());
    cstLoadHandler.load();
  }

  if (!cil_fname_.empty())
    design_.set_property(cil_filename, cil_fname_);
}

} // namespace RT
} // namespace FDU