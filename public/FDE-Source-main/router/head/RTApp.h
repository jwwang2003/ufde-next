#ifndef _RTAPP_H
#define _RTAPP_H

#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>

#include "netlist.hpp"
#include "rutils.h"
#include "tnetlist.hpp"

namespace FDU {
namespace RT {

namespace po = boost::program_options;
class Router;

struct RTParams {
  double first_iter_pres_fac;
  double initial_pres_fac;
  double pres_fac_mult;
  double hist_fac;
  int max_iters;
  int bb_fac;
  double max_crit;
  double crit_exp;
  double astar_fac;

  RTAlgorithm rt_algo;
  BaseCostType bcost_type;

  explicit RTParams(RTAlgorithm rt_algo_ = DIRECTED_SEARCH,
                    BaseCostType bcost_type_ = DEMAND_ONLY,
                    double first_iter_pres_fac_ = 0.5,
                    double initial_pres_fac_ = 0.5, double pres_fac_mult_ = 2.,
                    double hist_fac_ = 1., int max_iters_ = 30, int bb_fac_ = 3,
                    double max_crit_ = 0.99, double crit_exp_ = 1.,
                    double astar_fac_ = 0.5)
      : first_iter_pres_fac(first_iter_pres_fac_),
        initial_pres_fac(initial_pres_fac_), pres_fac_mult(pres_fac_mult_),
        hist_fac(hist_fac_), max_iters(max_iters_), bb_fac(bb_fac_),
        max_crit(max_crit_), crit_exp(crit_exp_), astar_fac(astar_fac_),
        rt_algo(rt_algo_), bcost_type(bcost_type_) {}
};

class RTApp {
public:
  RTApp();
  ~RTApp() {}

  void parse_command(int, char *[]);
  void try_process();

private:
  void store_cmd(int argc, char *argv[], po::variables_map &vm);
  void parse_vm(po::variables_map &vm);
  void set_default_output_file_name();
  bool file_extension_is_xml(const std::string &file_name) const;
  void set_algorithm(po::variables_map &vm);
  void load_files();
  void error_check();
  void display_usage_and_exit() const;

private:
  std::string arch_fname_;
  std::string netlist_fname_;
  std::string output_fname_;
  std::string cst_fname_;
  std::string sim_fname_; // revised
  std::string cil_fname_; // revised
  bool encrypted_;

  po::options_description desc_;

  COS::TDesign design_;

  RTParams rt_params_;
  Router *router_;
};

} // namespace RT
} // namespace FDU

#endif