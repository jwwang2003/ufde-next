#include <boost/range/adaptors.hpp>
#include <iostream>

#include "RTMsg.h"
#include "RTNode.h"
#include "Router.h"
#include "archValidation.h"
#include "rheap.h"
#include "simpleArchWriter.h"

namespace FDU {
namespace RT {

using namespace std;
using namespace boost;
using FDU::RRG::RRGNode;

Router Router::instance_;

Router::Router()
    : design_(nullptr), top_cell_(nullptr), rrg_(new RTGraph()),
      _pCstNets(nullptr) {}

Router::~Router() {
  delete rrg_;
  if (_pCstNets)
    delete _pCstNets;
}

void Router::initialize(TDesign *design, const RTParams &rt_params) {
  // initialize members
  design_ = design;
  if (_pCstNets) {
    ASSERT(design_->name() == _pCstNets->getDesignName(),
           "the design name in your netlist file and constraint file is "
           "different...");
  }
  top_cell_ = static_cast<RTCell *>(design->top_module());
  rt_params_ = rt_params;

  // initialize InfoPool
  info_pool_.set_design(design);
  info_pool_.set_rrg(rrg_);

  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_BUILD_DS);
  // for debug
  time_t rrg_begin, rrg_end;
  time(&rrg_begin);

  RRGFactory::set_factory(new RTNodeFactory());
  rrg_->build_rrg();
  set_rrgnodes_base_cost_index();

#ifdef DUMP_ARCH
  SimpleArchWriter archWriter;
  archWriter.writeArch();
#endif
  time(&rrg_end);
  FDU_LOG(INFO) << "  * Building rrg complete. Elapsed Time "
                << difftime(rrg_end, rrg_begin);
// #define CHECK_ARCH
#ifdef CHECK_ARCH
  ArchValidation archVal;
  archVal.validate();
#endif
  add_base_cost_info();

  // set bb_fac, source and sink(s) for every net
  for (RTNet *net : top_cell_->nets())
    net->init_rt_info(rrg_, rt_params_.bb_fac, _pCstNets);

  ASSERT(!info_pool_.check_netlist(), "");
  info_pool_.echo_netlist();
  info_pool_.echo_arch();
}

bool Router::try_route(TDesign *design, const RTParams &rt_params) {
  initialize(design, rt_params);

  int is_success;
  if (rt_params_.rt_algo == BREADTH_FIRST) {
    FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_RT_DESIGN) % "breadth-first";
    is_success = try_breadth_first_and_directd_search_route();
  } else if (rt_params_.rt_algo == DIRECTED_SEARCH) {
    FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_RT_DESIGN) % "directed-search";
    is_success = try_breadth_first_and_directd_search_route();
  } else if (rt_params_.rt_algo == TIMING_DRIVEN) {
    FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_RT_DESIGN) % "timing-driven";
    is_success = try_timing_driven_route();
  }
  if (is_success)
    info_pool_.echo_success_rt(is_success);

  return is_success;
}

void Router::reset_modify_nodes() {
  vector<RTNode *>::iterator node_it = mdf_nodes_.begin(),
                             end_node = mdf_nodes_.end();
  for (; node_it != end_node; ++node_it) {
    (*node_it)->reset_path_cost();
    (*node_it)->reset_total_cost();
  }
  mdf_nodes_.clear();
}

void Router::reset_is_path_nodes(RTNet &net) {
  RTNet::path_node_iter node_it;
  RTNet::path_node_iter end_node = net.path_end();

  for (node_it = net.path_begin(); node_it != end_node; ++node_it)
    (*node_it)->set_pathnode(false);
}

void Router::update_nodes_cost(double pres_fac, double his_fac) {
  for (RTNode *node : rrg_->nodes()) {
    int occ = node->occ();
    int cap = node->capacity();
    if (occ > cap) {
      node->inc_his_cost((occ - cap) * his_fac);
      node->set_pres_cost(1. + (occ + 1 - cap) * pres_fac);
    } else if (occ == cap)
      node->set_pres_cost(1. + pres_fac);
  }
}

int Router::infeasible_route() {
  int num_infeasible = 0;
  for (RTNet *net : top_cell_->nets()) {
    if (!net->is_feasible()) {
      ++num_infeasible;
    }
  }
  return num_infeasible;
}

#define LLENGTH 7
#define SEG_TYPE_NUM 3
void Router::set_rrgnodes_base_cost_index() {
  for (RRGNode *node : rrg_->nodes()) {
    RTNode *rt_node = static_cast<RTNode *>(node);
    switch (rt_node->type()) {
    case RRGNode::SOURCE:
    case RRGNode::CLK:
    case RRGNode::SINK:
      rt_node->set_timing(0., 0. + node->Cext());
      rt_node->set_base_cost_idx(rt_node->type());
      break;
    case RRGNode::SINGLE:
    case RRGNode::HEX:
      rt_node->set_timing(rt_node->R() * rt_node->length() * 0.5,
                          rt_node->C() * rt_node->length() * 0.5 +
                              rt_node->Cext());
      rt_node->set_base_cost_idx(rt_node->direction() * SEG_TYPE_NUM +
                                 rt_node->type());
      break;
    case RRGNode::LONG:
      rt_node->set_timing(rt_node->R() * LLENGTH,
                          rt_node->C() * LLENGTH + rt_node->Cext());
      rt_node->set_base_cost_idx(rt_node->direction() * SEG_TYPE_NUM +
                                 rt_node->type());
      break;
    default:
      ASSERT(0, (ErrMsg(ErrMsg::RTERR_NODE_TYPE) % rt_node->owner_name() %
                 rt_node->from_pos() % rt_node->to_pos()));
      break;
    }
  }
}
void Router::add_base_cost_info() {
  for (int i = SOURCE_COST_INDEX; i <= SINK_COST_INDEX; ++i)
    base_cost_infos_.push_back(BaseCostInfo());

  int index;
  const int SINGLE_LEN = 1;
  const int HEX_LEN = 6;

  int nx = FPGADesign::instance()->scale().x; // rows
  int ny = FPGADesign::instance()->scale().y; // cols

  // x channel
  for (int i = 1; i <= SEG_TYPE_NUM; ++i) {
    base_cost_infos_.push_back(BaseCostInfo());
    index = SEGX_COST_INDEX_START - 1 + i;
    // need to be complemented
    switch (i) {
    case 1:
      base_cost_infos_[index].inv_length = 1. / SINGLE_LEN;
      break;
    case 2:
      base_cost_infos_[index].inv_length = 1. / HEX_LEN;
      break;
    case 3:
      base_cost_infos_[index].inv_length = 1. / nx;
      break;
    default:
      base_cost_infos_[index].inv_length = OPEN;
      break;
    }
  }

  // y channel
  for (int i = 1; i <= SEG_TYPE_NUM; ++i) {
    base_cost_infos_.push_back(BaseCostInfo());
    // need to be complemented
    index = SEGX_COST_INDEX_START - 1 + SEG_TYPE_NUM + i;
    switch (i) {
    case 1:
      base_cost_infos_[index].inv_length = 1. / SINGLE_LEN;
      break;
    case 2:
      base_cost_infos_[index].inv_length = 1. / HEX_LEN;
      break;
    case 3:
      base_cost_infos_[index].inv_length = 1. / ny;
      break;
    default:
      base_cost_infos_[index].inv_length = OPEN;
      break;
    }
  }

  load_base_cost_TValue();

  save_base_cost();
}

void Router::load_base_cost_TValue() {
  FPGADesign *arch_fpga = FPGADesign::instance();
  int nx = arch_fpga->scale().x; // rows
  int ny = arch_fpga->scale().y; // cols

  int cost_idx;
  vector<int> num_cost_type(base_cost_infos_.size(), 0);
  vector<double> C_total(base_cost_infos_.size(), 0.);
  vector<double> R_total(base_cost_infos_.size(), 0.);

  // get typical GRM at position(nx/2-1, ny/2-1)
  const Point pos(Point(nx / 2 - 1, ny / 2 - 1));
  ArchCell *tile = arch_fpga->get_inst_by_pos(pos)->down_module();
  ArchCell::inst_iter grm_it =
      boost::find_if(tile->instances(), [this](const ArchInstance *inst) {
        return rrg_->is_grm(inst);
      });
  if (grm_it != tile->instances().end()) {
    for (Net *net : grm_it->down_module()->nets()) {
      RTNode *rt_node =
          static_cast<RTNode *>(rrg_->find_grm_net_node(net->name(), pos));
      if (rt_node->type() == RRGNode::SOURCE ||
          rt_node->type() == RRGNode::SINK)
        continue;

      cost_idx = rt_node->get_base_cost_idx();
      num_cost_type[cost_idx]++;
      C_total[cost_idx] += rt_node->C();
      R_total[cost_idx] += rt_node->R();
    }
  }

  double C_average, R_average;
  for (cost_idx = SEGX_COST_INDEX_START; cost_idx < base_cost_infos_.size();
       ++cost_idx) {
    if (num_cost_type[cost_idx] == 0) {
      base_cost_infos_[cost_idx].TLinear = OPEN;
      base_cost_infos_[cost_idx].TQuadratic = OPEN;
      base_cost_infos_[cost_idx].CLoad = OPEN;
    } else {
      C_average = C_total[cost_idx] / num_cost_type[cost_idx];
      R_average = R_total[cost_idx] / num_cost_type[cost_idx];

      // need to be complemented
      const double R_sw = 7.0e-1;
      const double Tdel_sw = 1.0e-1;

      base_cost_infos_[cost_idx].TLinear = Tdel_sw + R_sw * C_average;
      base_cost_infos_[cost_idx].TQuadratic =
          (R_sw + R_average) * 0.5 * C_average;
      base_cost_infos_[cost_idx].CLoad = C_average;
    }
  }
}

void Router::save_base_cost() {
  double delay_norm_fac;
  if (rt_params_.bcost_type == DELAY_NORMALIZED)
    delay_norm_fac = get_delay_norm_fac();
  else
    delay_norm_fac = 1.0;

  if (rt_params_.bcost_type == DELAY_NORMALIZED ||
      rt_params_.bcost_type == DEMAND_ONLY) {
    base_cost_infos_[SOURCE_COST_INDEX].base_cost = delay_norm_fac;
    base_cost_infos_[CLK_COST_INDEX].base_cost =
        delay_norm_fac / 2; // special for gclk
    base_cost_infos_[SINK_COST_INDEX].base_cost = 0.;
  } else if (rt_params_.bcost_type == INTRINSTIC_DELAY) {
    base_cost_infos_[SOURCE_COST_INDEX].base_cost = 0.;
    base_cost_infos_[CLK_COST_INDEX].base_cost = 0.5;
    base_cost_infos_[SINK_COST_INDEX].base_cost = 0.;
  }

  size_t max_idx = base_cost_infos_.size();
  for (size_t i = SEGX_COST_INDEX_START; i < max_idx; ++i) {
    if (rt_params_.bcost_type == INTRINSTIC_DELAY)
      base_cost_infos_[i].base_cost =
          base_cost_infos_[i].TLinear + base_cost_infos_[i].TQuadratic;
    else {
      base_cost_infos_[i].base_cost = 1;
    }
  }

  // save saved base cost
  for (int i = SOURCE_COST_INDEX; i < max_idx; ++i) {
    base_cost_infos_[i].saved_base_cost = base_cost_infos_[i].base_cost;
  }
}

double Router::get_delay_norm_fac() {
  const int clb_average_dist = 3;
  double Tdel_sum = 0, frac_num_seg, Tdel;

  FPGADesign *arch_fpga = FPGADesign::instance();
  int nx = arch_fpga->scale().x; // rows
  int ny = arch_fpga->scale().y; // cols

  // get typical GRM at position(nx/2-1, ny/2-1)
  const Point pos(Point(nx / 2 - 1, ny / 2 - 1));
  ArchCell *tile = arch_fpga->get_inst_by_pos(pos)->down_module();
  ArchCell::inst_iter grm_it =
      boost::find_if(tile->instances(), [this](const ArchInstance *inst) {
        return rrg_->is_grm(inst);
      });

  if (grm_it != tile->instances().end()) {
    for (Net *net : grm_it->down_module()->nets()) {
      RTNode *rt_node =
          static_cast<RTNode *>(rrg_->find_grm_net_node(net->name(), pos));
      if (rt_node->type() == RRGNode::SOURCE ||
          rt_node->type() == RRGNode::SINK || rt_node->type() == RRGNode::CLK)
        continue;

      int cost_idx = rt_node->get_base_cost_idx();
      frac_num_seg = clb_average_dist * base_cost_infos_[cost_idx].inv_length;
      Tdel =
          frac_num_seg * base_cost_infos_[cost_idx].TLinear +
          frac_num_seg * frac_num_seg * base_cost_infos_[cost_idx].TQuadratic;
      Tdel_sum += Tdel / static_cast<double>(clb_average_dist);
    }
  }

  return (Tdel_sum / static_cast<double>(grm_it->down_module()->num_nets()));
}

//////////////////////////////////////////////////////////////////////////
// for clk nets timing requirement
void Router::modify_base_cost_infos() {
  // GCLK
  base_cost_infos_[CLK_COST_INDEX].base_cost =
      base_cost_infos_[CLK_COST_INDEX].saved_base_cost * 0.5;
  // single
  base_cost_infos_[SEGX_COST_INDEX_START].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START].saved_base_cost * 2;
  base_cost_infos_[SEGX_COST_INDEX_START + SEG_TYPE_NUM].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + SEG_TYPE_NUM].saved_base_cost *
      2;
  // hex
  base_cost_infos_[SEGX_COST_INDEX_START + 1].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + 1].saved_base_cost;
  base_cost_infos_[SEGX_COST_INDEX_START + 1 + SEG_TYPE_NUM].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + 1 + SEG_TYPE_NUM]
          .saved_base_cost;
  // long
  base_cost_infos_[SEGX_COST_INDEX_START + 2].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + 2].saved_base_cost;
  base_cost_infos_[SEGX_COST_INDEX_START + 2 + SEG_TYPE_NUM].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + 2 + SEG_TYPE_NUM]
          .saved_base_cost;
}
void Router::reset_base_cost_infos() {
  // GCLK
  base_cost_infos_[CLK_COST_INDEX].base_cost =
      base_cost_infos_[CLK_COST_INDEX].saved_base_cost;
  // single
  base_cost_infos_[SEGX_COST_INDEX_START].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START].saved_base_cost;
  base_cost_infos_[SEGX_COST_INDEX_START + SEG_TYPE_NUM].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + SEG_TYPE_NUM].saved_base_cost;
  // hex
  base_cost_infos_[SEGX_COST_INDEX_START + 1].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + 1].saved_base_cost;
  base_cost_infos_[SEGX_COST_INDEX_START + 1 + SEG_TYPE_NUM].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + 1 + SEG_TYPE_NUM]
          .saved_base_cost;
  // long
  base_cost_infos_[SEGX_COST_INDEX_START + 2].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + 2].saved_base_cost;
  base_cost_infos_[SEGX_COST_INDEX_START + 2 + SEG_TYPE_NUM].base_cost =
      base_cost_infos_[SEGX_COST_INDEX_START + 2 + SEG_TYPE_NUM]
          .saved_base_cost;
}

} // namespace RT
} // namespace FDU