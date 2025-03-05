#ifndef _ROUTER_H
#define _ROUTER_H

#include "InfoPool.h"
#include "RTApp.h"
#include "RTCstLoadHandler.h"
#include "rheap.h"

namespace FDU {
namespace RT {

class RTNode;
class RTGraph;
class Router {
public:
  ~Router();

  bool try_route(TDesign *, const RTParams &);
  static Router &Instance() { return instance_; }
  std::vector<BaseCostInfo> &get_base_cost_infos() { return base_cost_infos_; }
  void modify_base_cost_infos();
  void reset_base_cost_infos();

  CstNets *getCstNets() {
    if (!_pCstNets) {
      _pCstNets = new CstNets();
    }
    return _pCstNets;
  }

  RTGraph *get_rrg() { return rrg_; }
  RTCell *get_rtcell() { return top_cell_; }
  string get_cil_fname() { return cil; }

private:
  Router();

  void set_rrgnodes_base_cost_index();
  void initialize(TDesign *, const RTParams &);
  int infeasible_route();
  void update_nodes_cost(double, double);
  void reset_modify_nodes();
  void add_to_modify_nodes(RTNode *p_node) { mdf_nodes_.push_back(p_node); }
  void add_base_cost_info();
  void load_base_cost_TValue();
  void save_base_cost();
  double get_delay_norm_fac();

  /////////////////////////////////////////////////////////////////////////////
  /// breadth first and directed search route
  int try_breadth_first_and_directd_search_route();
  bool breadth_first_and_directed_search_route_net(RTNet &);
  void expand_trace_back(RTNet::path_node_iter, RTNet &, BFHeap &);
  void reset_is_path_nodes(RTNet &);
  bool while_loop_should_not_end(RTNode *curr, RTNode *snk);

  //////////////////////////////////////////////////////////////////////////
  /// timing driven route
  int try_timing_driven_route();
  bool timing_driven_route_net(RTNet &, double, double);
  void mark_timing_sinks(RTNet &, std::vector<Pin *> &);
  double set_pincrit(Pin &, double);

private:
  RTParams rt_params_;
  TDesign *design_;
  RTCell *top_cell_;
  RTGraph *rrg_;
  InfoPool info_pool_;

  CstNets *_pCstNets;
  string cil;

  std::vector<RTNode *> mdf_nodes_;
  std::vector<BaseCostInfo> base_cost_infos_;
  std::vector<RTNode *> path_nodes_;

  static Router instance_;
};
} // namespace RT
} // namespace FDU

#endif