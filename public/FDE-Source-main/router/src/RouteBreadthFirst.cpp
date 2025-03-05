#include "RTMsg.h"
#include "RTNode.h"
#include <boost/phoenix/core.hpp>
#include <iostream>

namespace FDU {
namespace RT {

using namespace std;
using namespace boost;

int Router::try_breadth_first_and_directd_search_route() {
  modify_base_cost_infos();

  double pres_fac = rt_params_.first_iter_pres_fac;

  for (int itry = 1; itry <= rt_params_.max_iters; ++itry) {
    for (RTNet *net : top_cell_->nets()) {
      if (net->is_ignore())
        continue;

      net->path_update_one_cost(net->path_begin(), -1, pres_fac);
      if (!breadth_first_and_directed_search_route_net(*net)) {
        cerr << ErrMsg(ErrMsg::RTERR_UNRT_NET) % net->name() << endl;
        return 0;
      }
      net->path_update_one_cost(net->path_begin(), 1, pres_fac);
    }

    int num_infeasible = infeasible_route();
    if (num_infeasible == 0) {
      for (RTNet *net : top_cell_->nets())
        net->save_path();
      return itry;
    } else
      FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_ECHO_NUM_URT_NET) % itry %
                           num_infeasible;

    if (itry == 1)
      pres_fac = rt_params_.initial_pres_fac;
    else
      pres_fac *= rt_params_.pres_fac_mult;

    update_nodes_cost(pres_fac, rt_params_.hist_fac);
  }

  FDU_LOG(ERR) << ErrMsg(ErrMsg::RTERR_CONG_UNRESL);
  return 0;
}

bool Router::breadth_first_and_directed_search_route_net(RTNet &net) {
  net.free_path_back();
  if (net.is_partial_routed()) {
    FDU_LOG(INFO) << WarMsg(WarMsg::NLTWNG_PARTIAL_ROUTED) % net.name();
    net.load_info_from_fixed_path();
  }
  const RTNet::BBFac &bb = net.bb_fac();

  RTNode *src_node = net.src_node();
  net.mark_sinks();
  RTNet::path_node_iter newiter = net.path_begin();

  for (RTNode *snk_node : net.sink_nodes()) {
    if (snk_node->get_has_found()) {
      continue;
    }
    BFHeap bf_heap;
    BFHeapNode *bf_heap_node;

    bf_heap.add_node_to_heap(src_node, nullptr, nullptr, src_node->cong_cost());
    expand_trace_back(newiter, net, bf_heap);

    bf_heap_node = bf_heap.heap_head();

    if (bf_heap_node == nullptr) {
      reset_modify_nodes();
      return false;
    }

    RTNode *curr_node = bf_heap_node->_owner;

    while (while_loop_should_not_end(curr_node, snk_node)) {
      double path_cost = curr_node->path_cost();
      double new_cost = bf_heap_node->_total_cost;

      if (path_cost > new_cost) {
        curr_node->update_prev_info(bf_heap_node->_prev_rt_node, new_cost);

        if (path_cost > 0.99 * HUGE_FLOAT)
          add_to_modify_nodes(curr_node);

        bf_heap.expand_neighbours(src_node, curr_node, snk_node, bb, new_cost,
                                  rt_params_.astar_fac);
      }

      bf_heap_node->release();
      bf_heap_node = bf_heap.heap_head();

      if (bf_heap_node == nullptr) {
        reset_modify_nodes();
        return false;
      }
      curr_node = bf_heap_node->_owner;
    }

    if (curr_node->path_cost() > 0.99 * HUGE_FLOAT)
      add_to_modify_nodes(curr_node);

    curr_node->update_prev_info(bf_heap_node->_prev_rt_node,
                                bf_heap_node->_total_cost);
    curr_node->dec_tarflg();
    newiter = net.update_trace_back(curr_node);

    bf_heap_node->release();
    reset_modify_nodes();
  }
  reset_is_path_nodes(net);
  reset_modify_nodes();

  return true;
}

bool Router::while_loop_should_not_end(RTNode *curr, RTNode *snk) {
  if (rt_params_.rt_algo == BREADTH_FIRST) {
    return curr->tarflg() == 0;
  } else {
    return curr != snk;
  }
}
void Router::expand_trace_back(RTNet::path_node_iter node_it, RTNet &net,
                               BFHeap &bf_heap) {
  if (node_it != net.path_end()) {
    if (net.type() == CLOCK)
      bf_heap.add_node_to_heap((*node_it), nullptr, nullptr, NO_PREVIOUS);
    else
      (*node_it)->update_prev_info(nullptr, HUGE_FLOAT);

    ++node_it;
  }

  while (node_it != net.path_end()) {
    if (net.type() == CLOCK)
      bf_heap.add_node_to_heap((*node_it), nullptr, nullptr, NO_PREVIOUS);
    else
      (*node_it)->update_prev_info(nullptr, HUGE_FLOAT);

    if ((*node_it)->is_sink()) {
      ++node_it;
      if (node_it == net.path_end())
        break;
    }
    ++node_it;
  }
}

} // namespace RT
} // namespace FDU