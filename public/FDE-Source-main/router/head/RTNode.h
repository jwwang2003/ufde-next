#ifndef RTNODE_H
#define RTNODE_H

#include "Router.h"
#include "object.hpp"
#include "rrg/rrg.hpp"
#include "rutils.h"

namespace FDU {
namespace RT {

class TreeNode;

using namespace FDU::RRG;
using COS::PtrVector;
using namespace COS;

class RTNode : public RRGNode {
public:
  struct RTInfo {
    int tarflg;
    RTNode *prev_node;
    RRGSwitch *prev_sw;

    double pres_cost;
    double hist_cost;
    double path_cost;
    /// add for timing driven routing
    double total_cost;

    RTInfo()
        : prev_node(nullptr), pres_cost(1.), hist_cost(1.), path_cost(HUGE_FLOAT),
          total_cost(HUGE_FLOAT), tarflg(0) {}
  };

public:
  RTNode(RRGArchNet *owner, int capacity = 1)
      : RRGNode(owner), capacity_(capacity), occ_(0),
        base_cost_idx_(INVALID_INDEX), is_sink_(false), is_src_(false),
        tree_node_(nullptr), base_cost(1.0), is_pathnode_(false),
        has_found_(false) {}

  int capacity() const { return capacity_; }

  int occ() const { return occ_; }
  void set_occ(int occ) { occ_ = occ; }
  void add_occ() { ++occ_; }
  void dec_occ() { --occ_; }

  bool is_sink() const { return is_sink_; }
  void set_sink() { is_sink_ = true; }
  void clear_sink() { is_sink_ = false; }
  bool is_src() const { return is_src_; }
  void set_src() { is_src_ = true; }
  void clear_src() { is_src_ = false; }

  void set_has_found() { has_found_ = true; }
  bool get_has_found() const { return has_found_; }

  void set_pathnode(bool val) { is_pathnode_ = val; }
  bool is_pathnode() const { return is_pathnode_; }

  int tarflg() const { return rt_info_.tarflg; }
  void inc_tarflg() { ++rt_info_.tarflg; }
  void dec_tarflg() { --rt_info_.tarflg; }

  void set_base_cost_idx(int base_cost_idx) { base_cost_idx_ = base_cost_idx; }
  int get_base_cost_idx() { return base_cost_idx_; }

  void inc_his_cost(double inc_cost) { rt_info_.hist_cost += inc_cost; }
  void set_pres_cost(double cost) { rt_info_.pres_cost = cost; }
  void update_pres_cost(double pres_fac) {
    if (occ_ < capacity_)
      rt_info_.pres_cost = 1.;
    else
      rt_info_.pres_cost = 1. + (occ_ + 1 - capacity_) * pres_fac;
  }
  void mod_pres_cost() { rt_info_.pres_cost = 1. + rt_info_.pres_cost; }
  void update_prev_info(RTNode *prev_node, double path_cost,
                        RRGSwitch *prev_sw = nullptr, double total_cost = 0.) {
    rt_info_.prev_node = prev_node;
    rt_info_.path_cost = path_cost;
    rt_info_.prev_sw = prev_sw;
    rt_info_.total_cost = total_cost;
  }
  void reset_path_cost() { rt_info_.path_cost = HUGE_FLOAT; }
  void reset_total_cost() { rt_info_.total_cost = HUGE_FLOAT; }
  double path_cost() const { return rt_info_.path_cost; }
  double total_cost() const { return rt_info_.total_cost; }
  double cong_cost() const {
    return Router::Instance()
               .get_base_cost_infos()[base_cost_idx_]
               .base_cost /*base_cost */
           * rt_info_.hist_cost * rt_info_.pres_cost;
  }

  RTNode *prev_node() const { return rt_info_.prev_node; }
  RRGSwitch *prev_sw() const { return rt_info_.prev_sw; }

  void set_tree_node(TreeNode *tree_node) { tree_node_ = tree_node; }
  TreeNode *tree_node() const { return tree_node_; }

private:
  int occ_;
  int capacity_;
  int base_cost_idx_;
  bool is_sink_;
  bool is_src_;
  bool has_found_;
  bool is_pathnode_;

  RTInfo rt_info_;

  TreeNode *tree_node_;
  //////////
public:
  double base_cost;
};

class RTGraph : public RRGraph {

public:
  typedef Nodes::typed<RTNode>::iterator node_iter;
  typedef Nodes::typed<RTNode>::const_iterator const_node_iter;
  typedef Nodes::typed<RTNode>::range nodes_type;
  typedef Nodes::typed<RTNode>::const_range const_nodes_type;

  nodes_type nodes() { return static_cast<nodes_type>(RRGraph::nodes()); }
  const_nodes_type nodes() const {
    return static_cast<const_nodes_type>(RRGraph::nodes());
  }
};

class RTNodeFactory : public RRGFactory {
public:
  RRGNode *make_rrgnode(RRGArchNet *owner) { return new RTNode(owner); }
};
} // namespace RT
} // namespace FDU

#endif