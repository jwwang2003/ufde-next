#include <iostream>

#include "InfoPool.h"
#include "RTFactory.h"
#include "RTMsg.h"
#include "RTNode.h"
#include "netlist.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/range/adaptors.hpp>

namespace FDU {
namespace RT {

using namespace std;
using boost::adaptors::filtered;

using namespace COS;

bool InfoPool::check_netlist() const {
  bool is_fatle = false;
  for (RTNet *net : static_cast<RTCell *>(p_design_->top_module())->nets()) {
    if (net->is_ignore())
      continue;
    if (net->sink_pins().size() == 0) {
      cerr << ErrMsg(ErrMsg::NLTERR_NO_SNK) % net->name() << endl;
      is_fatle = true;
    }
    if (net->source_pins().size() != 1) {
      cerr << ErrMsg(ErrMsg::NLTERR_NOT_SGL_SRC) % net->name() << endl;
      is_fatle = true;
    }
    for (const Pin *pin : net->pins())
      if (pin->is_mpin()) {
        cerr << WarMsg(WarMsg::NLTWNG_PORT_CONN) % pin->name() % net->name()
             << endl;
        is_fatle = true;
      }
  }
  return is_fatle;
}

void InfoPool::echo_netlist() const {
  int num_nets = p_design_->top_module()->num_nets();
  int num_sinks = 0;
  int num_ignore = 0;
  for (const RTNet *net :
       static_cast<RTCell *>(p_design_->top_module())->nets()) {
    num_sinks += net->sink_pins().size();
    num_ignore += boost::lexical_cast<int>(net->is_ignore());
  }

  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_DESIGN) % p_design_->name();
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_RSRC) % "Net" % num_nets;
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_RSRC) % "ignore net" % num_ignore;
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_RSRC) % "sink" % num_sinks;
}

void InfoPool::echo_arch() const {
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_CHIP) %
                       FPGADesign::instance()->name();
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_RESOURCE);
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_RSRC) % "segment" %
                       p_rrg_->num_nodes();
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_RSRC) % "pip" %
                       p_rrg_->num_pips();
}

void InfoPool::echo_success_rt(int num_try) const {
  int num_pips = 0;
  int num_non_pips = 0;
  for (RTNet *net : static_cast<RTCell *>(p_design_->top_module())->nets()) {
    num_pips += net->num_pips();
    num_non_pips += boost::lexical_cast<int>(net->num_pips() == 0);
  }
  double prop_pip = double(num_pips) / p_rrg_->num_pips() * 100.;
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_SUCES_RT) % num_try % prop_pip %
                       num_non_pips;
}
} // namespace RT
} // namespace FDU