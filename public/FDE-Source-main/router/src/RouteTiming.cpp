#include <boost/phoenix/core.hpp>
#include <iostream>
#include <math.h>

#include "RTMsg.h"
#include "Router.h"
#include "rtree.h"
#include "rutils.h"
#include "tengine/tengine.hpp"

namespace FDU {
namespace RT {

using namespace std;
using namespace boost;
using namespace COS;

Property<double> Crit;
Property<double> &Slack = create_temp_property<double>(COS::PIN, "slack");

bool isSmaller(const Pin *p1, const Pin *p2) {
  return p1->property_value(Crit) > p2->property_value(Crit);
}

int Router::try_timing_driven_route() {
  double pres_fac = rt_params_.first_iter_pres_fac;
  double Dmax = 1.;

  modify_base_cost_infos();

  design_->timing_analyse(TEngine::REBUILD);

  //////////////////////////////////////////////////////////////////////////
  /// set every sink pin's delay and slack
  for (RTNet *rt_net : top_cell_->nets()) {
    if (rt_net->is_ignore())
      continue;

    for (Pin *sink_pin : rt_net->sink_pins()) {
      /// is rt_net global?
      sink_pin->set_property(Slack, TPara::ZERO);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  /// routing iteration
  for (int itry = 1; itry <= rt_params_.max_iters; ++itry) {
    for (RTNet *rt_net : top_cell_->nets()) {
      if (rt_net->is_ignore())
        continue;

      if (!timing_driven_route_net(*rt_net, Dmax, pres_fac)) {
        cerr << ErrMsg(ErrMsg::RTERR_UNRT_NET) % rt_net->name() << endl;
        return 0;
      }
    }

    int num_infeasible = infeasible_route();
    if (num_infeasible == 0) {
      for (RTNet *net : top_cell_->nets())
        net->save_path();
      return itry;
    } else
      FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_NUM_URT_NET) % itry %
                           num_infeasible;

    if (itry == 1) {
      pres_fac = rt_params_.initial_pres_fac;
      update_nodes_cost(pres_fac, 0);
    } else {
      pres_fac *= rt_params_.pres_fac_mult;
      update_nodes_cost(pres_fac, rt_params_.hist_fac);
    }

    //////////////////////////////////////////////////////////////////////////
    /// update the max Delay
    Dmax = design_->timing_analyse(TEngine::INCREMENT);
  }

  cerr << ErrMsg(ErrMsg::RTERR_CONG_UNRESL) << endl;
  return 0;
}

bool Router::timing_driven_route_net(RTNet &net, double Dmax, double pres_fac) {
  // 		if (net.type() == CLOCK)
  // 			modify_base_cost_infos();
  // 		else
  // 			reset_base_cost_infos();

  net.path_update_one_cost(net.path_begin(), -1, pres_fac);
  net.free_path_back();
  const RTNet::BBFac &bb = net.bb_fac();

  //////////////////////////////////////////////////////////////////////////
  /// set every sink pin's criticality
  double PinCrit;
  std::vector<Pin *> sink_pins; // for sort all sink pins
  for (Pin *sink_pin : net.sink_pins()) {
    PinCrit = set_pincrit(*sink_pin, Dmax);
    sink_pins.push_back(sink_pin);
  }
  sort(sink_pins.begin(), sink_pins.end(), isSmaller);

  //////////////////////////////////////////////////////////////////////////
  /// sort sink nodes
  mark_timing_sinks(net, sink_pins);

  //////////////////////////////////////////////////////////////////////////
  /// get source node
  RTNode *src_node = net.src_node();

  //////////////////////////////////////////////////////////////////////////
  /// creat routing tree(not routing heap)
  RouteTree RTree(src_node);

  for (int i = 0; i < net.sink_nodes().size(); ++i) {
    TDHeap td_heap;
    TDHeapNode *cur_heap_node;
    double TarCrit;
    RTNode *sink_node;

    Pin *sink_pin = sink_pins[i];
    TarCrit = sink_pin->property_value(Crit);
    sink_node = net.sink_nodes()[i];

    /// need to be complemented
    td_heap.add_rtree_to_heap(RTree.root(), *sink_pin, TarCrit,
                              rt_params_.astar_fac);

    cur_heap_node = td_heap.heap_head();

    if (cur_heap_node == nullptr) {
      reset_modify_nodes();
      return false;
    }

    RTNode *cur_rt_node = cur_heap_node->_owner;

    double old_path_cost, old_total_cost;
    double new_path_cost, new_total_cost;
    while (cur_rt_node != sink_node) {
      old_path_cost = cur_rt_node->path_cost();
      old_total_cost = cur_rt_node->total_cost();
      new_path_cost = cur_heap_node->_bpath_cost;
      new_total_cost = cur_heap_node->_total_cost;

      if (old_path_cost > new_path_cost && old_total_cost > new_total_cost) {
        if (old_total_cost > 0.99 * HUGE_FLOAT)
          add_to_modify_nodes(cur_rt_node);
        cur_rt_node->update_prev_info(cur_heap_node->_prev_rt_node,
                                      new_path_cost, cur_heap_node->_prev_sw,
                                      new_total_cost);

        td_heap.expand_neighbours(cur_heap_node, bb, TarCrit,
                                  rt_params_.astar_fac, *sink_pin);
      }

      cur_heap_node->release();
      cur_heap_node = td_heap.heap_head();

      if (cur_heap_node == nullptr) {
        reset_modify_nodes();
        return false;
      }

      cur_rt_node = cur_heap_node->_owner;
    }

    if (cur_rt_node->total_cost() > 0.99 * HUGE_FLOAT)
      add_to_modify_nodes(cur_rt_node);

    cur_rt_node->dec_tarflg();
    cur_rt_node->update_prev_info(cur_heap_node->_prev_rt_node, new_path_cost,
                                  cur_heap_node->_prev_sw, new_total_cost);

    RTNet::path_node_iter newiter = net.update_trace_back(cur_rt_node);

    RTree.update_route_tree(cur_heap_node);

    net.path_update_one_cost(newiter, 1, pres_fac);

    reset_modify_nodes();

    cur_heap_node->release();
  }
  RTree.update_net_delays_from_route_tree(&net, sink_pins);
  return true;
}

void Router::mark_timing_sinks(RTNet &net, std::vector<Pin *> &sink_pins) {
  net.sink_nodes().clear();
  std::vector<Pin *>::iterator sink_pin_it = sink_pins.begin();

  Property<Point> &postion =
      create_property<Point>(COS::INSTANCE, RT_CONST::POSITION);
  for (; sink_pin_it != sink_pins.end(); ++sink_pin_it) {
    Point pos = (*sink_pin_it)->instance()->property_value(postion);
    RTNode *sink_node = static_cast<RTNode *>(rrg_->find_logic_pin_node(
        (*sink_pin_it)->instance(), *(*sink_pin_it), pos));
    ASSERT(sink_node != nullptr, ErrMsg(ErrMsg::RTERR_UFND_NODE) % "sink" %
                                  (*sink_pin_it)->name() % net.name());

    sink_node->inc_tarflg();
    net.sink_nodes().push_back(sink_node);
  }
}

double Router::set_pincrit(Pin &p, double Dmax) {
  double PinCrit;
  double slack = p.property_value(Slack);
  PinCrit = max(rt_params_.max_crit - slack / Dmax, 0.);
  PinCrit = pow(PinCrit, rt_params_.crit_exp);
  PinCrit = min(PinCrit, rt_params_.max_crit);
  p.set_property(Crit, PinCrit);
  return PinCrit;
}
} // namespace RT
} // namespace FDU