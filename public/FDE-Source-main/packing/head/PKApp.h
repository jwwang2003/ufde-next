#ifndef _PKAPP_H
#define _PKAPP_H

#include <boost/program_options.hpp>

#include "PKMsg.h" //sophie
#include "Packer.h"
namespace PACK {

class PKApp {
public:
  PKApp() : fEncry_(true), desc_("Allow Options") {}

  static PKApp &instance() { return instance_; }

  void parse_command(int, char *[]);
  void try_process();

  const std::string &lib_fname() const { return lib_fname_; }
  const std::string &netlist_fname() const { return netlist_fname_; }
  const std::string &result_fname() const { return result_fname_; }
  const std::string &sim_fname() const { return sim_fname_; }
  const std::string &xdl_fname() const { return xdl_fname_; }
  const std::string &rpt_fname() const { return rpt_fname_; }

  Design &design() { return design_; }

private:
  void initialize();
  void display_usage();
  void cmd_error_check();

  /* Date:JULY 27th.09
  ---Author:Sophie
---Purpose:add information on design summary
  ---Function:design_summary();
  */
  void design_summary();

private:
  string netlist_fname_;
  string lib_fname_;
  string rule_fname_;
  string result_fname_;
  string sim_fname_;
  string xdl_fname_;
  string rpt_fname_;
  string config_fname_; // added by hz
  string log_fname_;    // added by hz

  bool fEncry_;

  boost::program_options::options_description desc_;
  Design design_;
  Packer packer_;

  static PKApp instance_;

  // string& chip_type_;
};
} // namespace PACK

#endif