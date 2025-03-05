#ifndef RRG_HPP
#define RRG_HPP

#include "arch/archlib.hpp"
#include "matrix.h"
#include "utils.h"

#include <boost/lexical_cast.hpp>
#include <memory>
#include <tuple>

namespace FDU {
namespace RRG {

using namespace ARCH;

//////////////////////////////////////////////////////////////////////////
// Parameters

namespace RRGPara {
const int NUM_SIDE_PORTS = 400;
const int NUM_TILE_SIDES = 4;
const int RAM_SPAN_RNG = 4;
const int UNDEF_INT = -1;
const double ZERO_DOUBLE = 0.;
const Point MIN_POS = Point();
const Point MAX_POS = Point(1000, 1000);
} // namespace RRGPara

//////////////////////////////////////////////////////////////////////////
// Class Declaration

class RRGArchInstance;
class RRGArchNet;
class RRGArchCell;
class RRGNode;
class RRGSwitch;

typedef RRGNode *RRGNodePtr;
typedef RRGNodePtr *PtrToRRGNodePtr;

//////////////////////////////////////////////////////////////////////////
// Memory Management

///// MemPool

template <typename T> class MemPool {
public:
  explicit MemPool(int init_size = 20000);
  ~MemPool();

  typedef T *TPtr;
  TPtr next();

  size_t size() const { return _mem_trunk.size(); }
  void reset() { _next_pos = 0; }

protected:
  void init(int init_size);
  void release();
  void resize(int size);

private:
  long _next_pos;

  typedef std::vector<TPtr> MemSeg;
  MemSeg _mem_seg;

  typedef std::vector<TPtr> MemTrunk;
  MemTrunk _mem_trunk;
};

//////////////////////////////////////////////////////////////////////////
// RRG Arch

///// RRGArchFactory

class RRGArchFactory : public ArchFactory {
public:
  Instance *make_instance(const string &name, Module *down_module,
                          Module *owner);
  Net *make_net(const string &name, NetType type, Module *owner, Bus *bus);
  Module *make_module(const string &name, const string &type, Library *owner);
};

///// RRGArchInstance

class RRGArchInstance : public ArchInstance {
public:
  RRGArchInstance(const string &name, Module *down_module, Module *owner)
      : ArchInstance(name, down_module, owner) {}

  typedef std::tuple<RRGArchNet *, RRGArchNet *, ArchPath *> Path;
  typedef PtrVector<Path> Paths;
  typedef Paths::iterator path_iter;
  typedef Paths::const_iterator const_path_iter;
  typedef boost::iterator_range<path_iter> paths_type;
  typedef boost::iterator_range<const_path_iter> const_paths_type;

  paths_type paths() { return paths_type(_paths.begin(), _paths.end()); }
  const_paths_type paths() const {
    return const_paths_type(_paths.begin(), _paths.end());
  }
  size_t num_paths() const { return _paths.size(); }

  Path *add_path(RRGArchNet *f_net, RRGArchNet *t_net, ArchPath *owner) {
    return _paths.add(new Path(f_net, t_net, owner));
  }

private:
  Paths _paths;
};

///// RRGArchNet

class RRGArchNet : public Net {
public:
  RRGArchNet(const std::string &name, NetType type, Module *owner, Bus *bus)
      : Net(name, type, owner, bus), _pptrs(), _index(RRGPara::UNDEF_INT) {}

  int index() const { return _index; }
  void set_index(int id) { _index = id; }

  RRGNodePtr rrg_node_ptr(int row) {
    PtrToRRGNodePtr pptr = ptr_to_rrg_node_ptr(row);
    return pptr ? *pptr : nullptr;
  }

  // return value nullptr means NOT be allocated with a pptr
  PtrToRRGNodePtr ptr_to_rrg_node_ptr(int row) {
    return _pptrs.count(row) ? _pptrs[row] : nullptr;
  }

  void set_ptr_to_rrg_node_ptr(int row, PtrToRRGNodePtr pptr) {
    _pptrs[row] = pptr;
  }

  void reset_pptrs() { _pptrs.clear(); }

private:
  typedef std::map<int, PtrToRRGNodePtr> PPtrs; // key<int> is row
  PPtrs _pptrs;

  long _index;
};

///// RRGArchCell

class RRGArchCell : public ArchCell {
public:
  RRGArchCell(const std::string &name, const std::string &type, Library *owner)
      : ArchCell(name, type, owner) {}

  typedef PtrList<Instance>::typed<RRGArchInstance>::iterator inst_iter;
  typedef PtrList<Instance>::typed<RRGArchInstance>::const_iterator
      const_inst_iter;
  typedef PtrList<Instance>::typed<RRGArchInstance>::range instances_type;
  typedef PtrList<Instance>::typed<RRGArchInstance>::const_range
      const_instances_type;

  instances_type instances() {
    return static_cast<instances_type>(Module::instances());
  }
  const_instances_type instances() const {
    return static_cast<const_instances_type>(Module::instances());
  }

  typedef PtrVector<Net>::typed<RRGArchNet>::iterator net_iter;
  typedef PtrVector<Net>::typed<RRGArchNet>::const_iterator const_net_iter;
  typedef PtrVector<Net>::typed<RRGArchNet>::range nets_type;
  typedef PtrVector<Net>::typed<RRGArchNet>::const_range const_nets_type;

  nets_type nets() { return static_cast<nets_type>(Module::nets()); }
  const_nets_type nets() const {
    return static_cast<const_nets_type>(Module::nets());
  }
};

//////////////////////////////////////////////////////////////////////////
// Routing Resource Graph (RRG)

///// RRGFactory

class RRGFactory {
public:
  typedef std::unique_ptr<RRGFactory> Pointer;

  static RRGFactory &instance() { return *_instance.get(); }
  static Pointer set_factory(Pointer &f) {
    Pointer p = std::move(_instance);
    _instance = std::move(f);
    return p;
  }
  static Pointer set_factory(RRGFactory *f) {
    Pointer p{f};
    return set_factory(p);
  }

  virtual ~RRGFactory() {}
  virtual RRGNode *make_rrgnode(RRGArchNet *owner);
  virtual RRGSwitch *make_rrgswitch(ArchPath *owner, RRGNode *from_node,
                                    RRGNode *to_node, const Point &pos);

private:
  static Pointer _instance;
};

///// RRGSwitch

class RRGSwitch {
public:
  enum RRG_SW_TYPE { DUMMY, PT, BUF };

  RRGSwitch(ArchPath *o, RRGNode *f, RRGNode *t, const Point &pos)
      : _pos(pos), _from_net(nullptr), _to_net(nullptr), _owner(o),
        _from_node(f), _to_node(t) {}
  virtual ~RRGSwitch() {}

  ArchPath *owner() const { return _owner; }
  const Point &pos() const { return _pos; }
  RRG_SW_TYPE type() const { return _type; }
  RRGNode *from_node() const { return _from_node; }
  RRGNode *to_node() const { return _to_node; }
  const Net *from_net() const { return _from_net; }
  const Net *to_net() const { return _to_net; }
  double R() const { return _owner->R(); }
  double Cin() const { return _owner->Cin(); }
  double Cout() const { return _owner->Cout(); }
  double delay() const { return _owner->delay(); }

  void set_from_net(Net *f_net) { _from_net = f_net; }
  void set_to_net(Net *t_net) { _to_net = t_net; }
  void set_type(const string &ctype) {
    _type = boost::lexical_cast<RRG_SW_TYPE>(ctype);
  }

private:
  Point _pos;
  Net *_from_net, *_to_net;
  RRG_SW_TYPE _type;
  ArchPath *_owner;
  RRGNode *_from_node, *_to_node;
};

std::istream &operator>>(std::istream &s, RRGSwitch::RRG_SW_TYPE &sw);
std::ostream &operator<<(std::ostream &s, RRGSwitch::RRG_SW_TYPE sw);

///// RRGNode

class RRGNode {
public:
  enum RRG_NODE_DIR { IGN_DIR = -1, HORIZONTAL, VERTICAL, NUM_OF_DIR };
  enum RRG_NODE_TYPE {
    IGN_TYPE = -1,
    SOURCE,
    CLK,
    SINK,
    SINGLE,
    HEX,
    LONG,
    NUM_OF_LEN
  };

  struct NodeTiming {
    double _R;
    double _C;    // total C = wire C + Cext
    double _Cext; // for C come from switches

    NodeTiming(double R = RRGPara::ZERO_DOUBLE, double C = RRGPara::ZERO_DOUBLE)
        : _R(R), _C(C), _Cext(RRGPara::ZERO_DOUBLE) {}
  };

  explicit RRGNode(RRGArchNet *owner, Point f_pos = RRGPara::MAX_POS)
      : _owner(owner), _from_pos(f_pos), _to_pos(RRGPara::MIN_POS) {}

  typedef PtrVector<RRGSwitch> Switches;
  typedef Switches::iterator switch_iter;
  typedef Switches::const_iterator const_switch_iter;
  typedef Switches::range_type switches_type;
  typedef Switches::const_range_type const_switches_type;

  switches_type switches() { return _switches.range(); }
  const_switches_type switches() const { return _switches.range(); }
  size_t num_switches() const { return _switches.size(); }

  string full_name() const;
  RRGArchNet *owner() const { return _owner; }
  string owner_name() const { return _owner->name(); }
  int length() const;                                 // { return _length; }
  RRG_NODE_DIR direction() const;                     // { return _dir; }
  RRG_NODE_TYPE type() const;                         // { return _type; }
  const Point &from_pos() const { return _from_pos; } // Top-Left Corner
  const Point &to_pos() const { return _to_pos; }     // Right-Bottom Corner
  double R() const { return _timing._R; }
  double C() const { return _timing._C; }
  double Cext() const { return _timing._Cext; }
  RRGSwitch *find_switch(RRGNode *t);

  void update_boundary(const Point &p);
  // void set_direciton	();
  // void set_length		();
  // void set_type		();
  void set_timing(double R, double C) {
    _timing._R = R;
    _timing._C = C;
  }
  void plus_Cext(double c_ext) { _timing._Cext += c_ext; }

  RRGSwitch *create_switch(ArchPath *o, RRGNode *f, RRGNode *t,
                           const Point &pos);

private:
  Switches _switches;
  RRGArchNet *_owner;
  // string			_owner_name;
  NodeTiming _timing;
  Point _from_pos, _to_pos;
  // int			_length;
  // RRG_NODE_DIR	_dir;
  // RRG_NODE_TYPE	_type;
};

///// RRGraph

class RRGraph {
public:
  typedef PtrVector<RRGNode> Nodes;
  typedef Nodes::iterator node_iter;
  typedef Nodes::const_iterator const_node_iter;
  typedef Nodes::range_type nodes_type;
  typedef Nodes::const_range_type const_nodes_type;

  nodes_type nodes() { return _nodes.range(); }
  const_nodes_type nodes() const { return _nodes.range(); }
  size_t num_nodes() const { return _nodes.size(); }
  int num_pips() const { return _info._num_pips; }

  void build_rrg();
  RRGNode *find_logic_pin_node(const Instance *inst, const Pin &pin,
                               const Point &pos) const;
  RRGNode *find_grm_net_node(const string &net_name, const Point &pos) const;
  bool is_grm(const Instance *inst) const;

protected:
  typedef MemPool<RRGNodePtr> NodePtrs;
  typedef std::vector<std::vector<PtrToRRGNodePtr>> SideNodePPtrs;
  typedef std::vector<SideNodePPtrs> ColNodePPtrs;

  bool is_ram(const Point &pos) const;
  void reset_grm_nodes(ArchCell &grm) const;
  void init_arch_info();
  void init_data_structure(NodePtrs ptrs[], ColNodePPtrs pptrs[]);
  void clear_node_ptrs(NodePtrs &node_ptrs);

  // Step 1: initialize data structures for GRM
  void init_grm_info(int cur_col);

  // Step 2: distinguish all the connected nets in a column,
  //		   and allocate a PtrToRRGNodePtr(RRGNode**) to each connected
  // net separately
  void spread_ptr_into_grm(int row, Pin &grm_pin, PtrToRRGNodePtr pptr);
  void spread_ptr_to_tile_port_and_grm(int row, ArchPort &cur_tile_port,
                                       SideNodePPtrs &cur_side_node_pptrs);
  void distinguish_nets_in_a_column(int col, NodePtrs &cur_node_ptrs,
                                    ColNodePPtrs &cur_col_node_pptrs);

  // Step 3: spread RRGNodes built in the last column to the current column
  void spread_nodes_from_last_column(int cur_col,
                                     ColNodePPtrs &last_col_node_pptrs,
                                     ColNodePPtrs &cur_col_node_pptrs);

  // Step 4: build RRGNodes for all the nets in the grm which
  // "*(PtrToRRGNodePtr) == nullptr"
  void build_grm(const Point &pos, ArchInstance &grm, NodePtrs &cur_node_ptrs);
  void build_grms_in_a_column(int cur_col, NodePtrs &cur_node_ptrs);

private:
  Nodes _nodes;

  typedef Matrix<std::vector<RRGNode *>> NodesLUT;
  NodesLUT _nodes_lut;

  // for details routing resource information
  struct RRGInfo {
    int _num_pips;
    Point _device_scale;

    RRGInfo() : _num_pips(0), _device_scale() {}
  };
  RRGInfo _info;
};

} // namespace RRG
} // namespace FDU

#endif