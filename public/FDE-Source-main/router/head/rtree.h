#ifndef RTREE_H
#define RTREE_H

#include "RTNode.h"
#include "utils.h"
#include <map>
#include <vector>

namespace FDU {
namespace RT {

class RTNet;
struct TDHeapNode;

struct TreeNode {
  RTNode *_owner;
  TreeNode *_parent;
  RRGSwitch *_parent_sw;
  bool _re_expand;
  double _Rupstream;
  double _Cdownstream;
  double _delay;

  typedef std::map<TreeNode *, RRGSwitch *> Children;
  typedef Children::value_type Child;

  Children _children;

  explicit TreeNode(RTNode *owner);
  ~TreeNode() {
    for (Child &child : _children) {
      delete child.first;
    }
  }
};

class RouteTree {
public:
  explicit RouteTree(RTNode *root) : _root(new TreeNode(root)) {}
  ~RouteTree() { delete _root; }

  // return a pointer to the tree node of the sink
  TreeNode *root() const { return _root; }
  TreeNode *update_route_tree(TDHeapNode *sink_hnode);
  // need to implement
  void update_net_delays_from_route_tree(RTNet *cur_net,
                                         std::vector<Pin *> &sink_pins);

protected:
  TreeNode *add_path_to_rout_tree(TDHeapNode *sink_hnode);
  void load_new_path_Rupstream(TreeNode *start_of_new_path);
  // return the root of the unbuffered subtree
  TreeNode *
  update_unbuffered_ancestors_Cdownstream(TreeNode *start_of_new_path);
  void load_unbuffered_subtree_delay(TreeNode *subtree_root, double tarr);

private:
  TreeNode *_root;
};

} // namespace RT
} // namespace FDU

#endif