#include "rtree.h"
#include "RTMsg.h"
#include "netlist.hpp"
#include "rheap.h"

namespace FDU {
namespace RT {

using namespace std;
using namespace COS;

//////////////////////////////////////////////////////////////////////////
// RTNode

TreeNode::TreeNode(RTNode *owner) : _owner(owner) {
  _parent = nullptr;
  _parent_sw = nullptr;
  _re_expand = true;
  _Rupstream = _Cdownstream = _delay = RRGPara::ZERO_DOUBLE;

  _owner->set_tree_node(this);
}

//////////////////////////////////////////////////////////////////////////
// RouteTree

TreeNode *RouteTree::update_route_tree(TDHeapNode *sink_hnode) {
  TreeNode *start_of_new_path = add_path_to_rout_tree(sink_hnode);
  TreeNode *sink_tree_node = sink_hnode->_owner->tree_node();

  load_new_path_Rupstream(start_of_new_path);

  TreeNode *unbuffered_subtree_root =
      update_unbuffered_ancestors_Cdownstream(start_of_new_path);

  double tarr = 0.;
  if (unbuffered_subtree_root->_parent) {
    tarr = unbuffered_subtree_root->_parent->_delay;
    tarr += unbuffered_subtree_root->_parent_sw->R() *
            unbuffered_subtree_root->_Cdownstream;
    tarr += unbuffered_subtree_root->_parent_sw->delay();
  }

  load_unbuffered_subtree_delay(unbuffered_subtree_root, tarr);

  return sink_tree_node;
}

void RouteTree::update_net_delays_from_route_tree(
    RTNet *cur_net, std::vector<Pin *> &sink_pins) {
  Property<TData> &DELAY = create_temp_property<TData>(COS::PIN, "delay");

  for (int i = 0; i < cur_net->sink_nodes().size(); ++i) {
    TreeNode *sink_tnode = (cur_net->sink_nodes()[i])->tree_node();
    sink_pins[i]->set_property(DELAY,
                               TData(sink_tnode->_delay, sink_tnode->_delay));
  }
}

TreeNode *RouteTree::add_path_to_rout_tree(TDHeapNode *sink_hnode) {
  double Cdownstream = 0.;

  TreeNode *sink_tree_node = new TreeNode(sink_hnode->_owner);
  Cdownstream = sink_tree_node->_owner->C();
  sink_tree_node->_Cdownstream = Cdownstream;
  sink_tree_node->_re_expand = false;
  TreeNode *downstream_tree_node = sink_tree_node;

  RTNode *current = sink_hnode->_prev_rt_node;
  RRGSwitch *child_sw = sink_hnode->_prev_sw;
  TreeNode *tree_node = nullptr;
  while (current->prev_node()) {
    tree_node = new TreeNode(current);
    tree_node->_re_expand = true;
    tree_node->_children.insert(make_pair(downstream_tree_node, child_sw));
    downstream_tree_node->_parent = tree_node;
    downstream_tree_node->_parent_sw = child_sw;

    if (child_sw->type() != RRGSwitch::BUF)
      Cdownstream += current->C();
    else
      Cdownstream = current->C();
    tree_node->_Cdownstream = Cdownstream;

    downstream_tree_node = tree_node;
    child_sw = current->prev_sw();
    current = current->prev_node();
  }

  tree_node = current->tree_node();
  tree_node->_children.insert(make_pair(downstream_tree_node, child_sw));
  downstream_tree_node->_parent = tree_node;
  downstream_tree_node->_parent_sw = child_sw;

  return downstream_tree_node;
}

void RouteTree::load_new_path_Rupstream(TreeNode *start_of_new_path) {
  TreeNode *tree_node = start_of_new_path;
  TreeNode *parent_tree_node = tree_node->_parent;
  RTNode *rt_node = tree_node->_owner;
  RRGSwitch *child_sw = tree_node->_parent_sw;

  double Rupstream = child_sw->R() + rt_node->R();
  if (child_sw->type() != RRGSwitch::BUF)
    Rupstream += parent_tree_node->_Rupstream;
  tree_node->_Rupstream = Rupstream;

  while (tree_node->_children.size()) {

    ASSERTD(tree_node->_children.size() == 1, ErrMsg(ErrMsg::RTERR_LD_RT));

    child_sw = tree_node->_children.begin()->second;
    rt_node = tree_node->_owner;

    if (child_sw->type() != RRGSwitch::BUF)
      Rupstream += child_sw->R() + rt_node->R();
    else
      Rupstream = child_sw->R() + rt_node->R();
    tree_node->_Rupstream = Rupstream;

    tree_node = tree_node->_children.begin()->first;
  }
}

TreeNode *RouteTree::update_unbuffered_ancestors_Cdownstream(
    TreeNode *start_of_new_path) {
  TreeNode *tree_node = start_of_new_path;
  TreeNode *parent_tree_node = tree_node->_parent;
  RRGSwitch *parent_sw = tree_node->_parent_sw;
  double Cdownstream_add = tree_node->_Cdownstream;

  while (parent_sw && parent_sw->type() != RRGSwitch::BUF) {
    tree_node = parent_tree_node;
    tree_node->_Cdownstream += Cdownstream_add;
    parent_tree_node = tree_node->_parent;
    parent_sw = tree_node->_parent_sw;
  }

  return tree_node;
}

void RouteTree::load_unbuffered_subtree_delay(TreeNode *subtree_root,
                                              double tarr) {
  RTNode *rt_node = subtree_root->_owner;
  double delay = tarr + 0.5 * subtree_root->_Cdownstream * rt_node->R();
  subtree_root->_delay = delay;

  for (TreeNode::Child &child : subtree_root->_children) {
    TreeNode *tree_node = child.first;
    RRGSwitch *child_sw = child.second;
    double child_delay =
        delay + child_sw->R() * tree_node->_Cdownstream + child_sw->delay();
    load_unbuffered_subtree_delay(tree_node, child_delay);
  }
}

} // namespace RT
} // namespace FDU
