#ifndef ROUTE_DELAY_TREE_HPP
#define ROUTE_DELAY_TREE_HPP

#include "iclib.hpp"
#include "rrg/rrg.hpp"
#include "rtnetlist.hpp"
#include "utils.h"
#include <boost/range.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>

using namespace FDU;
using namespace COS;
using namespace FDU::RRG;

namespace FDU {
namespace STA {

namespace RDPARA {
const double DEF_ARR_RISE = 0.;
const double DEF_ARR_FALL = DEF_ARR_RISE;
const double DEF_TRAN_RISE = 0.06;
const double DEF_TRAN_FALL = DEF_TRAN_RISE;
} // namespace RDPARA

struct RDData {
  RDData(double rise_, double fall_) : rise(rise_), fall(fall_) {}
  RDData() : rise(0.0), fall(0.0) {}

  void set_data(double rise_, double fall_) {
    rise = rise_;
    fall = fall_;
  }

  double rise;
  double fall;
};

class RDNode : public boost::noncopyable {

public:
  typedef std::vector<RDNode *> RDNodes;
  typedef RDNodes::iterator nodes_iter;
  typedef RDNodes::const_iterator const_nodes_iter;
  typedef boost::iterator_range<nodes_iter> nodes_type;
  typedef boost::iterator_range<const_nodes_iter> const_nodes_type;

  enum RDNODE_TYPE { INVALID_RDNODE, INPUT, OUTPUT, DUMMY, SWITCH, LINE };

  //! only valid for switch type
  //! node with line type is always considered as ON
  enum RDNODE_STATUS { INVALID_STATUS, OFF, ON };

  RDNode(const std::string &name, RDNODE_TYPE type, RDNode *parent,
         RRGSwitch *sw = 0)
      : name_(name), type_(type), status_(OFF), parent_(parent), switch_(sw),
        t_arr_(RDData(RDPARA::DEF_ARR_RISE, RDPARA::DEF_ARR_FALL)),
        t_tran_(RDData(RDPARA::DEF_TRAN_RISE, RDPARA::DEF_TRAN_FALL)),
        cap_in_(0.), cap_out_(0.) {}
  // virtual ~RDNode ();

  void add_child(RDNode *child) { children_.push_back(child); }
  void set_parent(RDNode *parent) { parent_ = parent; }

  void set_t_arr(double r, double f) { t_arr_.set_data(r, f); }
  void set_t_tran(double r, double f) { t_tran_.set_data(r, f); }
  void set_t_arr(const RDData &data) { t_arr_ = data; }
  void set_t_tran(const RDData &data) { t_tran_ = data; }
  void set_cap_in(double cap_in) { cap_in_ = cap_in; }
  void set_cap_out(double cap_out) { cap_out_ = cap_out; }

  void calc_cap();
  void calc_delay();

  ICPath *ic_path() const {
    if (type() == DUMMY) {
      return DummyICPath::dummy_icpath();
    }

    if (type() == LINE) {
      ICModule *module = ICLib::instance()->get_model("LINE");
      ASSERT(module, "cannot found module: "
                         << "LINE"
                         << " in STA Interconnect Lib");
      ICPath *path = module->get_path(0);
      ASSERTS(path);
      return path;
    }

    ArchPath *arch_path = rrg_sw()->owner();
    string icmod = arch_path->owner()->name();
    FDU_LOG(DEBUG) << "try find ic module: " << icmod;
    ICModule *module = ICLib::instance()->get_model(icmod);
    if (module == nullptr) {
      int debug = 0;
    }

    ASSERT(module,
           "cannot found module: " << icmod << " in STA Interconnect Lib");
    int path_idx = arch_path->type();
    if (path_idx > 0) {
      FDU_LOG(DEBUG)
          << "the fdp3 con lib is not integrity now, use arch path with 0 type";
      path_idx = 0;
    }
    ICPath *path = module->get_path(path_idx);
    ASSERTS(path);
    return path;
  }

  string icmod() const {
    ArchPath *arch_path = rrg_sw()->owner();
    return arch_path->owner()->name();
  }

  const std::string &name() const { return name_; }
  RDNODE_TYPE type() const { return type_; }
  RDNode *parent() const { return parent_; }
  RRGSwitch *rrg_sw() const { return switch_; }
  RDData t_arr() const { return t_arr_; }
  RDData t_tran() const { return t_tran_; }
  double cap_in() const { return cap_in_; }
  double cap_out() const { return cap_out_; }

  nodes_type children() {
    return nodes_type(children_.begin(), children_.end());
  }
  const_nodes_type children() const {
    return const_nodes_type(children_.begin(), children_.end());
  }
  auto num_children() const { return children_.size(); }

  bool is_root() const { return !parent(); }
  bool is_leave() const { return !num_children(); }

  RDNODE_STATUS status() const { return status_; }
  void set_status(RDNODE_STATUS status) { status_ = status; }
  void open() { set_status(ON); }
  void close() { set_status(OFF); }

  void recursive_print_cap();
  void recursive_print_timing();

  int index() const { return idx_; }
  void set_index(int i) { idx_ = i; }

private:
  void update_arr_and_tran(RDData &arr, RDData &tran);

  std::string name_;
  RDNODE_TYPE type_;
  RDNODE_STATUS status_;

  RDNode *parent_;
  RDNodes children_;

  RRGSwitch *switch_;

  RDData t_arr_;
  RDData t_tran_;

  double cap_in_;
  double cap_out_;

  int idx_; // for spice/dot dump
};

class RDTree : public boost::noncopyable {

public:
  typedef std::map<std::string, RDNode *> nodes_map;

  typedef std::map<RRGNode *, Pin *> seg_pin_map;
  typedef std::pair<RRGNode *, Pin *> seg_pin_pair;
  // typedef seg_pin_map::value_type                        seg_pin_pair;

  typedef std::map<RDNode *, Pin *> node_pin_map;
  typedef std::pair<RDNode *, Pin *> node_pin_pair;
  // typedef node_pin_map::value_type                       node_pin_pair;

  typedef std::vector<RRGNode *> segs;
  typedef std::map<RRGNode *, segs> seg_conn_map;
  typedef std::map<RRGNode *, bool> seg_visit_map;

  typedef boost::tuple<RRGNode *, RRGSwitch *, RRGNode *> Pip;
  typedef std::vector<Pip> Pips;
  typedef Pips::iterator pips_iter;
  typedef Pips::const_iterator const_pips_iter;
  typedef boost::iterator_range<pips_iter> pips_type;
  typedef boost::iterator_range<const_pips_iter> const_pips_type;

  RDTree(COSRTNet *owner) : owner_(owner) {}
  virtual ~RDTree() {
    for (nodes_map::iterator it = nodes_.begin(); it != nodes_.end(); ++it)
      delete it->second;
  }

  RDNode *find_node_by_name(const std::string &name) const {
    nodes_map::const_iterator it = nodes_.find(name);
    return it != nodes_.end() ? it->second : 0;
  }

  void post_build_process() {
    calc_all_nodes_load();
    calc_all_nodes_delay();
    set_delay_data();
  }

  RDNode *add_node(const std::string &name, RDNode::RDNODE_TYPE type,
                   RDNode *parent, RRGSwitch *sw = 0);

  void initial(const RRGraph &rrg) {
    build_src_node(rrg);
    build_snk_nodes(rrg);
    build_pips(rrg);
  }

  RDNode *root() { return src_node_.first; }
  Pin *driver_pin() { return src_seg_.second; }

  Pin *find_sink_pin_by_seg(RRGNode *seg) {
    seg_pin_map::iterator it = snk_segs_.find(seg);
    return it != snk_segs_.end() ? it->second : 0;
  }

  seg_pin_pair &src_seg() { return src_seg_; }
  const seg_pin_pair &src_seg() const { return src_seg_; }
  seg_pin_map &sink_segs() { return snk_segs_; }
  const seg_pin_map &sink_segs() const { return snk_segs_; }

  node_pin_pair &src_node() { return src_node_; }
  const node_pin_pair &src_node() const { return src_node_; }
  node_pin_map &sink_nodes() { return snk_nodes_; }
  const node_pin_map &sink_nodes() const { return snk_nodes_; }

  pips_type pips() { return pips_type(pips_.begin(), pips_.end()); }
  const_pips_type pips() const {
    return const_pips_type(pips_.begin(), pips_.end());
  }

  string switch_node_name_rule() const { return "%s_sw_%s_to_%s_at_%s"; }
  string line_node_name_rule() const { return "%s_line_%s_to_%s"; }

  // void    set_src_node_pin(RDNode* src_node, Pin* pin) { src_node_.first =
  // src_node; src_node_.second = pin; }
  void set_src_node_pin(RDNode *src_node, Pin *pin) {
    src_node_ = std::make_pair(src_node, pin);
  }
  void add_snk_node_pin(RDNode *snk_node, Pin *pin) {
    snk_nodes_.insert(std::make_pair(snk_node, pin));
  }

  COSRTNet *owner() const { return owner_; }

  //----------------------------------------------------------------------
  // dump related

  void dump_dot_file(const std::string &name);
  void dump_dot_node(RDNode *driver, std::ofstream &dot);
  void dump_spice_netlist(const std::string &name);
  void dump_spice_node(RDNode *driver, std::ofstream &sp, int &i);

protected:
  void calc_all_nodes_load();
  void calc_all_nodes_delay();
  void set_delay_data();

  void set_onode_cap_in();

  void build_src_node(const RRGraph &rrg);
  void build_snk_nodes(const RRGraph &rrg);
  void build_pips(const RRGraph &rrg);

  void dfs_visit_seg(RRGNode *node, seg_visit_map &nodes_repo,
                     seg_conn_map &conn_repo);

  seg_pin_pair src_seg_;
  seg_pin_map snk_segs_;
  node_pin_pair src_node_; //!< net source
  node_pin_map snk_nodes_; //!< net sinks
  nodes_map nodes_;        //!< nodes map
  COSRTNet *owner_;        //!< net
  Pips pips_;              //!< sorted pips, in reversed order
};

typedef std::shared_ptr<RDTree> RDTreePtr;

class RDFactory {
public:
  typedef std::unique_ptr<RDFactory> pointer;

  static RDFactory &instance() { return *_instance.get(); }
  static pointer set_factory(pointer f) {
    pointer p = std::move(_instance);
    _instance = std::move(f);
    return p;
  }
  static pointer set_factory(RDFactory *f) { return set_factory(pointer(f)); }

  virtual ~RDFactory() {}
  virtual RDNode *make_node(const std::string &name, RDNode::RDNODE_TYPE type,
                            RDNode *parent, RRGSwitch *sw = 0);
  virtual RDTreePtr make_tree(COSRTNet *net);

private:
  static pointer _instance;
};

inline RDNode *RDTree::add_node(const std::string &name,
                                RDNode::RDNODE_TYPE type, RDNode *parent,
                                RRGSwitch *sw /* = 0 */) {
  // ASSERTD(!nodes_.count(name), "node already exists: " + name);
  if (sw && sw->type() == RRGSwitch::DUMMY)
    type = RDNode::DUMMY;
  nodes_map::iterator it = nodes_.find(name);
  if (it == nodes_.end()) {
    RDNode *node = RDFactory::instance().make_node(name, type, parent, sw);
    if (parent)
      parent->add_child(node);
    nodes_.insert(make_pair(name, node));
    node->set_index(nodes_.size()); // make sure index > 0
    FDU_LOG(VERBOSE) << "add node: " << name;
    return node;
  }
  // allow repeated nodes for there maybe not only one pip at the same pos in
  // one net FDU_LOG(DEBUG) << "node already exists: " + name << std::endl;
  return it->second;
}

} // namespace STA
} // namespace FDU

#endif
