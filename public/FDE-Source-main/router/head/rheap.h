#ifndef RHEAP_H
#define RHEAP_H

#include "RTFactory.h"
#include "rutils.h"
#include <algorithm>
#include <vector>

namespace FDU {
namespace RRG {
class RRGSwitch;
}
} // namespace FDU

namespace FDU {
namespace RT {

using namespace FDU::RRG;

class RTNode;
class TreeNode;

struct HeapNode {
  RTNode *_owner;
  RTNode *_prev_rt_node;
  RRGSwitch *_prev_sw;

  double _total_cost;

  //		HeapNode(RTNode* owner, RTNode* prev_rt_node, RRGSwitch*
  // prev_sw, double tcost) 			: _owner(owner),
  // _prev_rt_node(prev_rt_node), _prev_sw(prev_sw), _total_cost(tcost)
  //		{}
  void assign(RTNode *owner, RTNode *prev_rt_node, RRGSwitch *prev_sw,
              double tcost) {
    _owner = owner;
    _prev_rt_node = prev_rt_node;
    _prev_sw = prev_sw;
    _total_cost = tcost;
  }
};

class Heap {
public:
  //		~Heap() { for(HeapNode* n: _heap) delete n; }

  HeapNode *heap_head();
  void add_node_to_heap(HeapNode *node);

protected:
  std::vector<HeapNode *> _heap;
};

//////////////////////////////////////////////////////////////////////////
// breadth-first heap

struct BFHeapNode : public HeapNode {
  //		BFHeapNode(RTNode* owner, RTNode* prev_n, RRGSwitch* prev_sw,
  // double tcost) 			: HeapNode(owner, prev_n, prev_sw,
  // tcost) {}
  static BFHeapNode *create(RTNode *owner, RTNode *prev_n, RRGSwitch *prev_sw,
                            double tcost);
  void release();
};

class BFHeap : public Heap {
public:
  ~BFHeap(); // { clear(); }

  void clear() {
    for (HeapNode *n : _heap)
      static_cast<BFHeapNode *>(n)->release();
    _heap.clear();
  }
  BFHeapNode *heap_head() {
    return static_cast<BFHeapNode *>(Heap::heap_head());
  }
  void add_node_to_heap(RTNode *cur_n, RTNode *prev_n, RRGSwitch *prev_sw,
                        double cur_total_cost);

  void expand_neighbours(RTNode *, RTNode *, RTNode *, const RTNet::BBFac &,
                         double, double);
  int get_expect_cost(RTNode *, RTNode *, RTNode *);
  int distance_between_points(const Point &, const Point &);
  int distance_between_nodes(const RTNode *, const RTNode *);
};

//////////////////////////////////////////////////////////////////////////
// timing-driven heap

struct TDHeapNode : public HeapNode {
  double _bpath_cost;
  double _Rupstream;

  //		TDHeapNode(RTNode* owner, RTNode* prev_n, RRGSwitch* prev_sw, double tcost, \
//					double bp_cost, double Rup)
  //			: HeapNode(owner, prev_n, prev_sw, tcost),
  //_bpath_cost(bp_cost), _Rupstream(Rup)
  //		{}

  void assign(RTNode *owner, RTNode *prev_n, RRGSwitch *prev_sw, double tcost,
              double bp_cost, double Rup) {
    HeapNode::assign(owner, prev_n, prev_sw, tcost);
    _bpath_cost = bp_cost;
    _Rupstream = Rup;
  }

  static TDHeapNode *create(RTNode *owner, RTNode *prev_n, RRGSwitch *prev_sw,
                            double tcost, double bp_cost, double Rup);
  void release();
};

class TDHeap : public Heap {
public:
  ~TDHeap() {
    for (HeapNode *n : _heap)
      static_cast<TDHeapNode *>(n)->release();
  }
  TDHeapNode *heap_head() {
    return static_cast<TDHeapNode *>(Heap::heap_head());
  }
  void add_node_to_heap(RTNode *cur_n, RTNode *prev_n, RRGSwitch *prev_sw,
                        double cur_total_cost, double cur_bp_cost, double Rup);

  void expand_neighbours(TDHeapNode *, const RTNet::BBFac &, double, double,
                         const Pin &);
  double get_expected_cost(RTNode *, const Pin &, double, double);
  int get_exp_segs_num(RTNode *, const Pin &, int &);
  void add_rtree_to_heap(TreeNode *, const Pin &, double, double);
  void empty_heap(TDHeapNode *);
};

//////////////////////////////////////////////////////////////////////////
// inline functions

inline bool total_cost_smaller(const HeapNode *lhs, const HeapNode *rhs) {
  return lhs->_total_cost > rhs->_total_cost;
}

inline void Heap::add_node_to_heap(HeapNode *node) {
  _heap.push_back(node);
  std::push_heap(_heap.begin(), _heap.end(), total_cost_smaller);
}

} // namespace RT
} // namespace FDU

#endif