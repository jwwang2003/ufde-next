#ifndef TENGINE_HPP
#define TENGINE_HPP

#include "object.hpp"
#include <fstream>
#include <set>

namespace COS {

namespace TPara {
const int DEFAULT_TDOMAINS = 1;
const double ZERO = 0.;
const double MAXIMUM_FLOAT = 1000.;
const double MINIMUM_FLOAT = -MAXIMUM_FLOAT;
const double T_CONST_GEN = MINIMUM_FLOAT;
} // namespace TPara

template <typename T>
struct Less //: public std::binary_function<T, T, bool>
{
  bool operator()(const T *lhs, const T *rhs) const {
    return lhs->less_than(*rhs);
  }
};

// with ownership
template <typename T, typename L = Less<T>> using TESet = PtrSet<T, L, true>;

// without ownership
template <typename T, typename L = Less<T>>
using SharedTESet = PtrSet<T, L, false>;

class Pin;
class Port;
class Instance;
class Module;
class TDesign;
class TNode;
class TEdge;

struct TData {
  double _rising;
  double _falling;

  TData(double r = TPara::ZERO, double f = TPara::ZERO)
      : _rising(r), _falling(f) {}

  double average() const { return (_rising + _falling) / 2.; }
  void set_value(double rising, double falling) {
    _rising = rising;
    _falling = falling;
  }
};

class TElement : boost::noncopyable {
public:
  bool less_than(const TElement &e) const { return this < &e; }

  //		virtual DOM::Element* create_xml(DOM::Element* root)
  // const = 0; 		virtual void		echo_dotfile(const
  // TNode& from_node, std::ofstream& ofs) const = 0;
};

class TEdge : public TElement {
public:
  TEdge(TNode *f_tnode, TNode *t_tnode, double r_delay, double f_delay)
      : _from_node(f_tnode), _to_node(t_tnode), _delay(r_delay, f_delay) {}
  virtual ~TEdge() {}

  bool less_than(const TEdge &e) const {
    return this->average_delay() != e.average_delay()
               ? this->average_delay() < e.average_delay()
               : TElement::less_than(e);
  }
  double r_delay() const { return _delay._rising; }
  double f_delay() const { return _delay._falling; }
  double average_delay() const { return _delay.average(); }
  TNode *from_node() const { return _from_node; }
  TNode *to_node() const { return _to_node; }
  void set_delay(const TData &delay) { _delay = delay; }

  //		virtual void		echo_dotfile(const TNode& from_node,
  // std::ofstream& ofs) const;

private:
  TNode *_from_node, *_to_node;
  TData _delay;
};

class TNode : public TElement {
public:
  enum TNodeType {
    INVALID_TYPE = -1,
    IN_PORT,  /*IN_PAD,*/
    OUT_PORT, /*OUT_PAD,*/
    CELL_IPIN,
    CELL_OPIN,
    CELL_CLKPIN,
    SEQ_SRC,
    SEQ_SINK,
    CONST_GEN_SRC,
    NUM_TNODETYPE
  };

  TNode(TNodeType type, Pin *owner, int index)
      : _visited(false), _index(index), _type(type), _owner(owner) {}
  virtual ~TNode() {}

  typedef TESet<TEdge> OutTEdgeSet;
  typedef OutTEdgeSet::iterator otedge_iter;
  typedef OutTEdgeSet::const_iterator const_otedge_iter;
  typedef OutTEdgeSet::range_type otedges_type;
  typedef OutTEdgeSet::const_range_type const_otedges_type;

  otedges_type out_tedges() { return _out_tedges.range(); }
  const_otedges_type out_tedges() const { return _out_tedges.range(); }
  size_t num_out_tedges() const { return _out_tedges.size(); }

  typedef SharedTESet<TEdge> InTEdgeSet;
  typedef InTEdgeSet::iterator itedge_iter;
  typedef InTEdgeSet::const_iterator const_itedge_iter;
  typedef InTEdgeSet::range_type itedges_type;
  typedef InTEdgeSet::const_range_type const_itedges_type;

  itedges_type in_tedges() { return _in_tedges.range(); }
  const_itedges_type in_tedges() const { return _in_tedges.range(); }
  size_t num_in_tedges() const { return _in_tedges.size(); }

  typedef std::vector<TData> TDataVec;
  typedef TDataVec::iterator tdata_iter;
  typedef TDataVec::const_iterator const_tdata_iter;
  typedef boost::iterator_range<tdata_iter> tdata_type;
  typedef boost::iterator_range<const_tdata_iter> const_tdata_type;

  tdata_type t_arrs() {
    return tdata_type(_t_arr_vec.begin(), _t_arr_vec.end());
  }
  const_tdata_type t_arrs() const {
    return const_tdata_type(_t_arr_vec.begin(), _t_arr_vec.end());
  }
  tdata_type t_reqs() {
    return tdata_type(_t_req_vec.begin(), _t_req_vec.end());
  }
  const_tdata_type t_reqs() const {
    return const_tdata_type(_t_req_vec.begin(), _t_req_vec.end());
  }
  size_t num_tdata() const { return _t_arr_vec.size(); }
  TData t_arr(int t_idx) const { return _t_arr_vec[t_idx]; }
  void resize_tdata(int i) {
    _t_arr_vec.resize(i, TData(TPara::MINIMUM_FLOAT, TPara::MINIMUM_FLOAT));
    _t_req_vec.resize(i, TData(TPara::MAXIMUM_FLOAT, TPara::MAXIMUM_FLOAT));
  }

  bool less_than(const TNode &n) const { return this->index() < n.index(); }
  int index() const { return _index; }
  TNodeType type() const { return _type; }
  Pin *owner() const { return _owner; }
  bool is_visited() const { return _visited; }
  void set_visited() { _visited = true; }
  void clr_visited() { _visited = false; }

  TEdge *create_out_edge(TNode *t_tnode, double r_delay = TPara::ZERO,
                         double f_delay = TPara::ZERO);

  // re-implement virtual functions for more details
  typedef std::map<int, TData> IntTDataMap;
  typedef IntTDataMap::value_type IntTDataPair;

  virtual void update_pinput_tarrs(int t_index);
  virtual void update_poutput_treqs(const IntTDataMap &max_delays);
  virtual void update_normal_tarrs(const TEdge *f_tedge);
  virtual void update_normal_treqs(const TEdge *t_tedge);

  //		virtual void		echo_dotfile(const TNode& from_node,
  // std::ofstream& ofs) const;

private:
  bool _visited;
  int _index;
  TNodeType _type;
  Pin *_owner;
  InTEdgeSet _in_tedges;
  OutTEdgeSet _out_tedges;
  TDataVec _t_arr_vec, _t_req_vec;
};

class TFactory {
public:
  typedef std::unique_ptr<TFactory> Pointer;

  static TFactory &instance() { return *_instance.get(); }
  static Pointer set_factory(Pointer f) {
    Pointer p = std::move(_instance);
    _instance = std::move(f);
    return p;
  }
  static Pointer set_factory(TFactory *f) { return set_factory(Pointer(f)); }

  virtual ~TFactory() {}
  virtual TNode *make_tnode(TNode::TNodeType type, Pin *owner, int index);
  virtual TEdge *make_tedge(TNode *f_tnode, TNode *t_tnode, double r_delay,
                            double f_delay);

private:
  static Pointer _instance;
};

struct TimingInfo;
class TEngine {
public:
  enum WORK_MODE { REBUILD, INCREMENT };

  explicit TEngine(TDesign *t = nullptr)
      : _target(t), _t_domains(TPara::DEFAULT_TDOMAINS) {}
  virtual ~TEngine() {}

  typedef TESet<TNode> TNodeSet;
  typedef TNodeSet::iterator tnode_iter;
  typedef TNodeSet::const_iterator const_tnode_iter;
  typedef TNodeSet::range_type tnodes_type;
  typedef TNodeSet::const_range_type const_tnodes_type;

  tnodes_type tnodes() { return _tnodes.range(); }
  const_tnodes_type tnodes() const { return _tnodes.range(); }
  size_t num_tnodes() const { return _tnodes.size(); }

  typedef std::vector<TData> TDataVec;
  typedef TDataVec::iterator tdata_iter;
  typedef TDataVec::const_iterator const_tdata_iter;
  typedef boost::iterator_range<tdata_iter> tdata_type;
  typedef boost::iterator_range<const_tdata_iter> const_tdata_type;

  tdata_type max_delays() {
    return tdata_type(_max_delay_vec.begin(), _max_delay_vec.end());
  }
  const_tdata_type max_delays() const {
    return const_tdata_type(_max_delay_vec.begin(), _max_delay_vec.end());
  }
  const TData &chief_max_delay() const { return *max_delays().begin(); }
  double average_max_delay() const { return chief_max_delay().average(); }

  double timing_analyse(WORK_MODE mode);
  void set_target_design(TDesign *d) { _target = d; }
  TDesign *target_design() const { return _target; }

protected:
  typedef PtrVector<TNode, false> TNodeVec;
  typedef TNodeVec::iterator sorted_tnode_iter;
  typedef TNodeVec::const_iterator const_sorted_tnode_iter;
  typedef TNodeVec::range_type sorted_tnodes_type;
  typedef TNodeVec::const_range_type const_sorted_tnodes_type;

  sorted_tnodes_type sorted_tnodes() { return _sorted_tnodes.range(); }
  const_sorted_tnodes_type sorted_tnodes() const {
    return _sorted_tnodes.range();
  }
  size_t num_sorted_tnodes() const { return _sorted_tnodes.size(); }

  void load_rt_delay();
  void create_tgraph();
  void compute_tgraph();

  void create_all_nodes();
  void create_module_pin_node(Pin *mpin);
  void create_inst_nodes(Instance *inst);
  void create_composite_inst_pin_node(Pin *pin);
  void create_primitive_inst_pin_node(Pin *pin);
  TNode *create_node(TNode::TNodeType type, Pin *owner,
                     bool set_pin_prop = true);

  void create_all_conns();
  void create_inst_conns(Instance *inst);
  void create_composite_inst_conns(Instance *inst);
  void create_primitive_inst_conns(Instance *inst);
  void create_conn(Pin *src, Pin *sink);

  void topo_sort();
  void topo_visit(TNode *node);

  void initial_tdata();
  void compute_tarr();
  void compute_treq();
  void compute_slack();

  int t_domains() const { return _t_domains; }
  void set_t_domains(int num) { _t_domains = num; }

  TData find_primitive_inst_delay(Pin *rel_pin, string TimingInfo::*const tp,
                                  const string &t_value);

  // re-implement virtual functions for more details
  virtual void update_max_delays(TNode *cur_tnode);
  virtual void update_slack_to_pin(TEdge *tedge) const;

  typedef std::map<int, TData> IntTDataMap;

  virtual int t_domain_index(const TNode *node) const; // only for primary input
  virtual IntTDataMap
  t_domain_index_map(const TNode *node) const; // only for primary output

private:
  bool _is_built;
  TDesign *_target;
  int _t_domains;
  TDataVec _max_delay_vec;
  TNodeSet _tnodes;
  TNodeVec _sorted_tnodes;
};

///////////////////////////////////////////////////////////////////////////////////////
//  TimingInfo
struct TimingInfo {
  string timing_type;
  string related_pin;
  string timing_sense;
  double intrinsic_rise;
  double intrinsic_fall;

  TimingInfo() : intrinsic_rise(0.0), intrinsic_fall(0.0) {}
  TimingInfo(const string &type, const string &pin, const string &sense,
             double rise, double fall)
      : timing_type(type), related_pin(pin), timing_sense(sense),
        intrinsic_rise(rise), intrinsic_fall(fall) {}
};

const string SETUP_RISING = "setup_rising";
const string RISING_EDGE = "rising_edge";

} // namespace COS

#endif