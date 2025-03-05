#include <boost/pool/pool.hpp>

#include "Router.h"
#include "property.hpp"
#include "rheap.h"
#include "rtree.h"
#include "rutils.h"

namespace FDU {
namespace RT {

using namespace std;
using FDU::RRG::RRGNode;

//////////////////////////////////////////////////////////////////////////
// Heap

HeapNode *Heap::heap_head() {
  if (_heap.size()) {
    // move the element with *smallest* total_cost to the back of the vector
    pop_heap(_heap.begin(), _heap.end(), total_cost_smaller);
    // get the *smallest* from the back
    HeapNode *smallest = _heap.back();
    // remove "smallest"
    _heap.pop_back();
    return smallest;
  } else // empty
    return nullptr;
}

template <typename T> class SimplePool {
  vector<pair<T *, int>> store;
  int next_size;
  T *chunk;
  int chunk_size;
  int chunk_idx, block_idx;

public:
  SimplePool(int size = 32)
      : next_size(size), chunk(0), chunk_size(-1), chunk_idx(-1),
        block_idx(-1) {}
  ~SimplePool() {
    for (int i = 0; i < store.size(); ++i)
      delete[] store[i].first;
  }
  T *malloc() {
    if (++block_idx >= chunk_size) {     // chunk full
      if (++chunk_idx >= store.size()) { // alloc new chunk
        chunk_size = next_size;
        chunk = new T[chunk_size];
        store.push_back(make_pair(chunk, chunk_size));
        next_size *= 2;
      } else {
        chunk = store[chunk_idx].first;
        chunk_size = store[chunk_idx].second;
      }
      block_idx = 0;
    }
    return chunk + block_idx;
  }
  void free(T *) {} // do nothing
  void reset() { chunk_size = chunk_idx = block_idx = -1; }
};

//////////////////////////////////////////////////////////////////////////
// BFHeap

//	static boost::object_pool<BFHeapNode> bf_pool;
//	static boost::pool<> bf_pool(sizeof(BFHeapNode));
static SimplePool<BFHeapNode> bf_pool(65536);
BFHeap::~BFHeap() { bf_pool.reset(); }

BFHeapNode *BFHeapNode::create(RTNode *owner, RTNode *prev_n,
                               RRGSwitch *prev_sw, double tcost) {
  BFHeapNode *node = static_cast<BFHeapNode *>(bf_pool.malloc());
  node->assign(owner, prev_n, prev_sw, tcost);
  return node;
}
void BFHeapNode::release() { bf_pool.free(this); }

void BFHeap::add_node_to_heap(RTNode *cur_n, RTNode *prev_n, RRGSwitch *prev_sw,
                              double cur_total_cost) {
  if (cur_total_cost > /*=*/cur_n->path_cost())
    return;

  Heap::add_node_to_heap(
      BFHeapNode::create(cur_n, prev_n, prev_sw, cur_total_cost));
}

void BFHeap::expand_neighbours(RTNode *src_node, RTNode *rt_node,
                               RTNode *snk_node, const RTNet::BBFac &bb,
                               double path_cost, double astar_fac) {
  RTNode *child_node;
  double total_cost;

  for (RRGSwitch *sw : rt_node->switches()) {
    child_node = static_cast<RTNode *>(sw->to_node());

    if (child_node->from_pos().x > bb.bbrb.x ||
        child_node->from_pos().y > bb.bbrb.y ||
        child_node->to_pos().x < bb.bblt.x ||
        child_node->to_pos().y < bb.bblt.y)
      continue;

    total_cost = path_cost + child_node->cong_cost();

    // when astar_fac != 0, directed search.
    if (astar_fac != 0.) {
      total_cost += astar_fac * get_expect_cost(src_node, child_node, snk_node);
    }

    if (child_node->is_pathnode())
      add_node_to_heap(child_node, nullptr, nullptr, total_cost);
    else
      add_node_to_heap(child_node, rt_node, sw, total_cost);
  }
}

int BFHeap::get_expect_cost(RTNode *src_node, RTNode *curr_node,
                            RTNode *snk_node) {
  // 		if(distance_between_nodes(src_node, snk_node) > 1)
  // 		{
  // 			return distance_between_nodes(curr_node, snk_node) /
  // distance_between_nodes(src_node, snk_node);
  // 		}
  // 		else
  return distance_between_nodes(curr_node, snk_node);
}

int BFHeap::distance_between_nodes(const RTNode *first_node,
                                   const RTNode *second_node) {
  Point first_from_pos(first_node->from_pos());
  Point first_to_pos(first_node->to_pos());
  Point second_from_pos(second_node->from_pos());
  Point second_to_pos(second_node->to_pos());

  int mim_distance = distance_between_points(first_from_pos, second_from_pos);
  int tmp_d1 = distance_between_points(first_from_pos, second_to_pos);
  int tmp_d2 = distance_between_points(first_to_pos, second_from_pos);
  int tmp_d3 = distance_between_points(first_to_pos, second_to_pos);
  mim_distance = mim_distance < tmp_d1 ? mim_distance : tmp_d1;
  mim_distance = mim_distance < tmp_d2 ? mim_distance : tmp_d2;
  mim_distance = mim_distance < tmp_d3 ? mim_distance : tmp_d3;

  return mim_distance;
}

int BFHeap::distance_between_points(const Point &first_pos,
                                    const Point &second_pos) {
  return abs(first_pos.x - second_pos.x) + abs(first_pos.y - second_pos.y);
}
//////////////////////////////////////////////////////////////////////////
// TDHeap

//	static boost::object_pool<TDHeapNode> td_pool;
static boost::pool<> td_pool(sizeof(TDHeapNode));

TDHeapNode *TDHeapNode::create(RTNode *owner, RTNode *prev_n,
                               RRGSwitch *prev_sw, double tcost, double bp_cost,
                               double Rup) {
  TDHeapNode *node = static_cast<TDHeapNode *>(td_pool.malloc());
  node->assign(owner, prev_n, prev_sw, tcost, bp_cost, Rup);
  return node;
}
void TDHeapNode::release() { td_pool.free(this); }

void TDHeap::add_node_to_heap(RTNode *cur_n, RTNode *prev_n, RRGSwitch *prev_sw,
                              double cur_total_cost, double cur_bp_cost,
                              double Rup) {
  if (cur_total_cost >= cur_n->total_cost())
    return;

  Heap::add_node_to_heap(TDHeapNode::create(cur_n, prev_n, prev_sw,
                                            cur_total_cost, cur_bp_cost, Rup));
}

void TDHeap::expand_neighbours(TDHeapNode *td_heap_node, const RTNet::BBFac &bb,
                               double TarCrit, double astar_fac,
                               const Pin &sink_pin) {
  RTNode *rt_node = td_heap_node->_owner;
  double Rupstream = td_heap_node->_Rupstream;
  RTNode *child_node;

  for (RRGSwitch *sw : rt_node->switches()) {
    child_node = static_cast<RTNode *>(sw->to_node());

    if (child_node->from_pos().x > bb.bbrb.x ||
        child_node->from_pos().y > bb.bbrb.y ||
        child_node->to_pos().x < bb.bblt.x ||
        child_node->to_pos().y < bb.bblt.y)
      continue;

    double NewRupstream = 0.;
    RRGSwitch::RRG_SW_TYPE sw_type = sw->type();
    if (sw_type == RRGSwitch::PT)
      NewRupstream = Rupstream + sw->R();
    else if (sw_type == RRGSwitch::BUF)
      NewRupstream = sw->R();

    //////////////////////////////////////////////////////////////////////////
    /// add current child node's R
    NewRupstream += child_node->R();

    //////////////////////////////////////////////////////////////////////////
    /// node n's delay Tdel(n) = Td(switch) + (Rupstream - R(n)/2)*C(n)
    /// Rupstream include current node n's R
    double Tdel =
        sw->delay() + (NewRupstream - child_node->R() / 2.) * child_node->C();

    //////////////////////////////////////////////////////////////////////////
    /// from source to child new_path_cost = old_path_cost + (1 - TarCrit) *
    /// p(n)b(n)h(n) + TarCrit * delay(n)
    double new_path_cost = td_heap_node->_bpath_cost +
                           (1 - TarCrit) * child_node->cong_cost() +
                           TarCrit * Tdel;

    //////////////////////////////////////////////////////////////////////////
    /// total_cost = path_cost + crit_fac * expected_cost
    double new_total_cost =
        new_path_cost; // + astar_fac * get_expected_cost(child_node, sink_pin,
                       // NewRupstream, TarCrit);

    add_node_to_heap(child_node, rt_node, sw, new_total_cost, new_path_cost,
                     NewRupstream);
  }
}

double TDHeap::get_expected_cost(RTNode *cur_rt_node, const Pin &tar_pin,
                                 double Rupstream, double TarCrit) {
  int num_segs_ortho_dir;
  int num_segs_same_dir =
      get_exp_segs_num(cur_rt_node, tar_pin, num_segs_ortho_dir);

  /// set base_cost = 1 temporary!
  double cong_cost = num_segs_same_dir * 1 + num_segs_ortho_dir * 1;
  double T_same_dir = 0.5 * cur_rt_node->R() * cur_rt_node->C();
  double T_ortho_dir = 0.5 * cur_rt_node->R() * cur_rt_node->C();

  double Tdel =
      num_segs_same_dir * T_same_dir + num_segs_ortho_dir * T_ortho_dir +
      Rupstream * cur_rt_node->C() * (num_segs_same_dir + num_segs_ortho_dir);

  return TarCrit * Tdel + (1 - TarCrit) * cong_cost;
}

int TDHeap::get_exp_segs_num(RTNode *cur_rt_node, const Pin &tar_pin,
                             int &num_segs_ortho_dir) {
  Property<Point> &postion =
      create_property<Point>(COS::INSTANCE, RT_CONST::POSITION);
  Point tar_pos = tar_pin.instance()->property_value(postion);
  Point cur_pos = (abs(cur_rt_node->from_pos().x - tar_pos.x) +
                   abs(cur_rt_node->from_pos().y - tar_pos.y)) >
                          (abs(cur_rt_node->to_pos().x - tar_pos.x) +
                           abs(cur_rt_node->to_pos().y - tar_pos.y))
                      ? cur_rt_node->to_pos()
                      : cur_rt_node->from_pos();

  RRGNode::RRG_NODE_DIR seg_dir = cur_rt_node->direction();
  int seg_length = cur_rt_node->length();

  if (seg_dir == RRGNode::RRG_NODE_DIR::HORIZONTAL) {
    num_segs_ortho_dir =
        ceil(static_cast<double>(abs(tar_pos.y - cur_pos.y) / seg_length));
    return ceil(abs(static_cast<double>(tar_pos.x - cur_pos.x) / seg_length));
  } else if (seg_dir == RRGNode::RRG_NODE_DIR::VERTICAL) {
    num_segs_ortho_dir =
        ceil(static_cast<double>(abs(tar_pos.x - cur_pos.x) / seg_length));
    return ceil(static_cast<double>(abs(tar_pos.y - cur_pos.y) / seg_length));
  } else {
    ///???????????????????
    num_segs_ortho_dir = 0;
    return 0;
  }
}

void TDHeap::add_rtree_to_heap(TreeNode *cur_root, const Pin &tar_pin,
                               double TarCrit, double astar_fac) {
  if (cur_root->_re_expand) {
    RTNode *cur_rt_node = cur_root->_owner;
    double path_cost = TarCrit * cur_root->_delay;
    double total_cost =
        path_cost; // + astar_fac * get_expected_cost(cur_rt_node, tar_pin,
                   // cur_root->_Rupstream, TarCrit);
    add_node_to_heap(cur_rt_node, nullptr, nullptr, total_cost, path_cost,
                     cur_root->_Rupstream);

    TreeNode::Children::iterator child_iter = cur_root->_children.begin();
    for (; child_iter != cur_root->_children.end(); ++child_iter) {
      add_rtree_to_heap(child_iter->first, tar_pin, TarCrit, astar_fac);
    }
  }
}
} // namespace RT
} // namespace FDU