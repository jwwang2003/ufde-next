#include "rrg.hpp"
#include "netlist.hpp"
#include <fstream>

#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>

namespace FDU {
namespace RRG {

using namespace std;
using namespace boost;

namespace LIBRARY {
const string PRIM = "primitive";
const string BLOCK = "block";
const string TILE = "tile";
} // namespace LIBRARY

namespace CELL {
const string GSB = "GSB";
const string GRM = "GRM";
} // namespace CELL

#undef ALLOW_DANGLING
#define DEVELOPING

#undef SPREAD_FROM_LEFT_TO_RIGHT
#define SPREAD_FROM_RIGHT_TO_LEFT
// 	#undef  SPREAD_FROM_RIGHT_TO_LEFT
// 	#define SPREAD_FROM_LEFT_TO_RIGHT

//////////////////////////////////////////////////////////////////////////
// Memory Management

///// MemPool

template <typename T> MemPool<T>::MemPool(int init_size /* = 20000 */) {
  init(init_size);
}

template <typename T> MemPool<T>::~MemPool() { release(); }

template <typename T> typename MemPool<T>::TPtr MemPool<T>::next() {
  if (_next_pos >= _mem_trunk.size())
    resize(_mem_trunk.size());

  return _mem_trunk[_next_pos++];
}

template <typename T> void MemPool<T>::init(int init_size) {
  _next_pos = 0;
  _mem_trunk.reserve(init_size);
  resize(init_size);
}

template <typename T> void MemPool<T>::release() {
  for (size_t i = 0; i < _mem_seg.size(); ++i)
    delete[] _mem_seg[i];
}

template <typename T> void MemPool<T>::resize(int size) {
  TPtr new_seg = new T[size];
  memset(new_seg, 0, size);
  _mem_seg.push_back(new_seg);

  for (int i = 0; i < size; ++i) {
    _mem_trunk.push_back(new_seg + i);
  }
}

//////////////////////////////////////////////////////////////////////////
// RRGArch

///// RRGArchFactory

Instance *RRGArchFactory::make_instance(const string &name, Module *down_module,
                                        Module *owner) {
  return new RRGArchInstance(name, down_module, owner);
}
Net *RRGArchFactory::make_net(const string &name, NetType type, Module *owner,
                              Bus *bus) {
  return new RRGArchNet(name, type, owner, bus);
}
Module *RRGArchFactory::make_module(const string &name, const string &type,
                                    Library *owner) {
  return new RRGArchCell(name, type, owner);
}

//////////////////////////////////////////////////////////////////////////
// Routing Resource Graph (RRG)

///// RRGFactory

RRGFactory::Pointer RRGFactory::_instance(new RRGFactory());
RRGNode *RRGFactory::make_rrgnode(RRGArchNet *owner) {
  return new RRGNode(owner);
}
RRGSwitch *RRGFactory::make_rrgswitch(ArchPath *owner, RRGNode *from_node,
                                      RRGNode *to_node, const Point &pos) {
  return new RRGSwitch(owner, from_node, to_node, pos);
}

///// RRGNode

string RRGNode::full_name() const {
  return str(format("%1%_from_%2%_to_%3%") % owner_name() %
             lexical_cast<string>(_from_pos) % lexical_cast<string>(_to_pos));
}

RRGSwitch *RRGNode::find_switch(RRGNode *t) {
  switch_iter sw_it = find_if(
      switches(), [t](const RRGSwitch *sw) { return sw->to_node() == t; });
  return sw_it == switches().end() ? nullptr : *sw_it;
}

// already adapt to Vertex2
void RRGNode::update_boundary(const FDU::Point &p) {
  int min_x = min(_from_pos.x, p.x);
  int min_y = min(_from_pos.y, p.y);
  _from_pos = Point(min_x, min_y);

  int max_x = max(_to_pos.x, p.x);
  int max_y = max(_to_pos.y, p.y);
  _to_pos = Point(max_x, max_y);
}

// need to change for Vertex2
RRGNode::RRG_NODE_DIR RRGNode::direction() const {
  return _from_pos == _to_pos       ? IGN_DIR
         : _from_pos.y != _to_pos.y ? HORIZONTAL
                                    : VERTICAL;
}

// need to change for Vertex2
int RRGNode::length() const {
  return direction() == HORIZONTAL ? abs(_to_pos.y - _from_pos.y)
         : direction() == VERTICAL ? abs(_to_pos.x - _from_pos.x)
                                   : 0;
}

// need to change for Vertex2
RRGNode::RRG_NODE_TYPE RRGNode::type() const {
  RRG_NODE_TYPE type;
  if (owner_name().find("GCLK") != string::npos)
    type = CLK;
  else if ((owner_name().find("LH") != string::npos ||
            owner_name().find("LV") != string::npos ||
            owner_name().find("LLH") != string::npos ||
            owner_name().find("LLV") != string::npos) &&
           length() != 0)
    type = LONG;
  else if (owner_name().find("H6") != string::npos ||
           owner_name().find("V6") != string::npos)
    type = HEX;
  else if (length() == 1 || length() == 2)
    type = SINGLE;
  else
    type = num_switches() ? SOURCE : SINK;
  return type;
}

///// RRGSwitch

static const char *sw_types[] = {"dummy", "pt", "buf"};
static EnumStringMap<RRGSwitch::RRG_SW_TYPE> sw_type_map(sw_types);
istream &operator>>(istream &s, RRGSwitch::RRG_SW_TYPE &sw) {
  sw = sw_type_map.readEnum(s);
  return s;
}
ostream &operator<<(ostream &s, RRGSwitch::RRG_SW_TYPE sw) {
  return sw_type_map.writeEnum(s, sw);
}

// const Point& RRGSwitch::pos() const {return _from_node->to_pos(); }

RRGSwitch *RRGNode::create_switch(ArchPath *o, RRGNode *f, RRGNode *t,
                                  const Point &pos) {
  RRGSwitch *sw = RRGFactory::instance().make_rrgswitch(o, f, t, pos);
  _switches.add(sw);
  return sw;
}

///// RRGraph

RRGNode *RRGraph::find_logic_pin_node(const Instance *inst, const Pin &pin,
                                      const Point &pos) const {
  int loop = is_ram(pos) ? RRGPara::RAM_SPAN_RNG : 1;
  // 		if(loop == 4)
  // 			int a = 0;
  for (int i = 0; i < loop; ++i) {
    Point t_pos(pos.x - i, pos.y, pos.z);
    ArchCell *tile_cell =
        FPGADesign::instance()->get_inst_by_pos(t_pos)->down_module();
    for (const ArchInstance *ainst : tile_cell->instances()) {
      if (ainst->module_type() != inst->module_type() ||
          ainst->z_pos() != pos.z)
        continue;
      const Pin *t_pin = ainst->find_pin(pin.name());
      if (t_pin == nullptr || t_pin->dir() != pin.dir())
        return nullptr;
      if (!t_pin->net())
        continue;
      for (const Pin *pin : t_pin->net()->pins()) {
        if (pin->is_mpin())
          continue;
        if (pin->instance()->module_type() == CELL::GRM ||
            pin->instance()->module_type() == CELL::GSB) {
          Net *net = pin->down_pin()->net();
#ifdef ALLOW_DANGLING
          if (!net)
            return nullptr;
#else
          ASSERTD(net, "dangling: grm port cell pin " + pin->name());
#endif
          return _nodes_lut.at(t_pos)[static_cast<RRGArchNet *>(net)->index()];
        }
      }
    }
  }
  return nullptr;
}

RRGNode *RRGraph::find_grm_net_node(const string &net_name,
                                    const Point &pos) const {
  int loop = is_ram(pos) ? RRGPara::RAM_SPAN_RNG : 1;
  for (int i = 0; i < loop; ++i) {
    Point t_pos(pos.x - i, pos.y, pos.z);
    ArchCell *tile_cell =
        FPGADesign::instance()->get_inst_by_pos(t_pos)->down_module();

    ArchCell::inst_iter grm_it =
        find_if(tile_cell->instances(),
                [this](const Instance *inst) { return is_grm(inst); });
    if (grm_it == tile_cell->instances().end())
      return nullptr;
    Net *net = grm_it->down_module()->find_net(net_name);
    if (!net)
      continue;
    return _nodes_lut.at(t_pos)[static_cast<RRGArchNet *>(net)->index()];
  }
  return nullptr;
}

bool RRGraph::is_grm(const Instance *inst) const {
  return inst->module_type() == CELL::GRM   ? true
         : inst->module_type() == CELL::GSB ? true
                                            : false;
}

bool RRGraph::is_ram(const Point &pos) const {
  return FPGADesign::instance()->is_bram(pos.y);
}

void RRGraph::reset_grm_nodes(ArchCell &grm) const {
  for (RRGArchNet *net : static_cast<RRGArchCell &>(grm).nets())
    net->reset_pptrs();
}

void RRGraph::init_arch_info() {
  Library *tile_lib = FPGADesign::instance()->find_library(LIBRARY::TILE);
  Library *prim_lib = FPGADesign::instance()->find_library(LIBRARY::PRIM);
  if (!prim_lib) {
    prim_lib = FPGADesign::instance()->find_library(LIBRARY::BLOCK);
  }
  ASSERTD(prim_lib, "arch error: primitive library not found.");

  for (Module *prim_cell : prim_lib->modules()) {
    if (prim_cell->type() != CELL::GRM && prim_cell->type() != CELL::GSB)
      continue;
    // for all nets in GRM
    int index = 0;
    for (RRGArchNet *net : static_cast<RRGArchCell *>(prim_cell)->nets())
      net->set_index(index++);

    // for all instances in GRM
    for (RRGArchInstance *ainst :
         static_cast<RRGArchCell *>(prim_cell)->instances()) {
      for (ArchPath *path : ainst->down_module()->paths()) {
        Pin *f_pin = ainst->find_pin(path->from_port()->name());
        Pin *t_pin = ainst->find_pin(path->to_port()->name());

        ASSERTD(f_pin && t_pin && f_pin->net() && t_pin->net(),
                "arch error: illegal path.");
        ainst->add_path(static_cast<RRGArchNet *>(f_pin->net()),
                        static_cast<RRGArchNet *>(t_pin->net()), path);
      }
    }
  }

  _info._device_scale = FPGADesign::instance()->scale();
  _nodes_lut.renew(_info._device_scale.x, _info._device_scale.y);
}

void RRGraph::init_data_structure(NodePtrs ptrs[], ColNodePPtrs pptrs[]) {
  for (int i = 0; i < 2; ++i) {
    clear_node_ptrs(ptrs[i]);
    pptrs[i].resize(_info._device_scale.x);
    for (int row = 0; row < _info._device_scale.x; ++row) {
      pptrs[i][row].resize(RRGPara::NUM_TILE_SIDES);
      for (int side = 0; side < RRGPara::NUM_TILE_SIDES; ++side)
        pptrs[i][row][side].resize(RRGPara::NUM_SIDE_PORTS, nullptr);
    }
  }
}

void RRGraph::clear_node_ptrs(NodePtrs &node_ptrs) {
  node_ptrs.reset();
  for (int i = 0, size = node_ptrs.size(); i < size; ++i)
    *node_ptrs.next() = nullptr;
  node_ptrs.reset();
}

void RRGraph::build_rrg() {
  init_arch_info();

  ColNodePPtrs pptrs[2];
  NodePtrs ptrs[2];
  init_data_structure(ptrs, pptrs);

  int last_idx = 0, curr_idx = 1;

#if defined(SPREAD_FROM_LEFT_TO_RIGHT)
  for (int col = 0; col < _info._device_scale.y; ++col) {
#elif defined(SPREAD_FROM_RIGHT_TO_LEFT)
  for (int col = _info._device_scale.y - 1; col >= 0; --col) {
#endif
    // Step 1
    init_grm_info(col);
    // Step 2
    distinguish_nets_in_a_column(col, ptrs[curr_idx], pptrs[curr_idx]);
    // Step 3
#if defined(SPREAD_FROM_LEFT_TO_RIGHT)
    if (col > 0)
#elif defined(SPREAD_FROM_RIGHT_TO_LEFT)
    if (col < _info._device_scale.y - 1)
#endif
      spread_nodes_from_last_column(col, pptrs[last_idx], pptrs[curr_idx]);
    // Step 4
    build_grms_in_a_column(col, ptrs[curr_idx]);

    swap(last_idx, curr_idx);
  }

  // need to change for Vertex2
  // foreach(RRGNode* node, nodes()){
  // node->set_direciton();
  // node->set_length();
  // node->set_type();
  //}
}

///// Shared by Steps

// used to improve speed
inline static ArchCell *&cur_col_tile(int row) {
  // Singleton Design Pattern to make sure initialization before used
  typedef vector<ArchCell *> CurColTiles;
  static CurColTiles cur_col_tiles(FPGADesign::instance()->scale().x, nullptr);

  return cur_col_tiles[row];
}

///// Step 1

void RRGraph::init_grm_info(int cur_col) {
  for (int row = 0; row < _info._device_scale.x; ++row) {
    Point curr_pos(row, cur_col);
    ArchCell *tile =
        FPGADesign::instance()->get_inst_by_pos(curr_pos)->down_module();
    cur_col_tile(row) = tile; // used to improve speed

    ArchCell::inst_iter grm_it =
        boost::find_if(tile->instances(),
                       [this](const Instance *inst) { return is_grm(inst); });
    if (grm_it != tile->instances().end()) {
      reset_grm_nodes(*((*grm_it)->down_module()));
      _nodes_lut.at(curr_pos).resize((*grm_it)->down_module()->num_nets(),
                                     nullptr);
    }
  }
}

///// Step 2

void RRGraph::spread_ptr_into_grm(int row, Pin &grm_pin, PtrToRRGNodePtr pptr) {
  RRGArchNet *grm_cpin_net =
      static_cast<RRGArchNet *>(grm_pin.down_pin()->net());
#ifdef ALLOW_DANGLING
  if (!grm_cpin_net) {
    cerr << "[WARNING] dangling: grm port cell pin" << grm_pin.name() << endl;
    continue;
  }
#else
  ASSERTD(grm_cpin_net, "dangling: grm port cell pin" + grm_pin.name());
#endif
  if (grm_cpin_net->ptr_to_rrg_node_ptr(row)) {
    ASSERTD(pptr == grm_cpin_net->ptr_to_rrg_node_ptr(row),
            "multi sources: multi tile ports to one grm port " +
                grm_pin.name());
  } else {
    grm_cpin_net->set_ptr_to_rrg_node_ptr(row, pptr);
  }
}

void RRGraph::spread_ptr_to_tile_port_and_grm(
    int row, ArchPort &cur_tile_port, SideNodePPtrs &cur_side_node_pptrs) {
  int pin_num = cur_tile_port.width();
  for (int index = 0; index < pin_num; index++) {
    PtrToRRGNodePtr node_pptr =
        cur_side_node_pptrs[cur_tile_port.side()]
                           [cur_tile_port.mpin(index)->index()];
    ASSERTD(node_pptr,
            cur_tile_port.name() + ": pptr of tile port is nullptr.");

    Pin *port_cpin = cur_tile_port.mpin(index);
    Net *cpin_net = port_cpin->net();
    // 			ASSERTD(cpin_net, rrg_error(string("dangling: tile port
    // ").append(cur_tile_port.name()).append(" unconnected.")));
    //			80k arch has some discontinous port serial number.

    if (cpin_net) {
      for (Pin *pin : cpin_net->pins()) {
        if (pin == port_cpin)
          continue;

        // connect to other tile ports
        if (pin->is_mpin()) {
          ArchPort *another_tile_port = static_cast<ArchPort *>(pin->port());
          cur_side_node_pptrs[another_tile_port->side()][pin->index()] =
              node_pptr;
        }
        // connect into GRM
        else if (pin->instance()->module_type() == CELL::GRM ||
                 pin->instance()->module_type() == CELL::GSB)
          spread_ptr_into_grm(row, *pin, node_pptr);
      }
    }
  }
}

void RRGraph::distinguish_nets_in_a_column(int cur_col, NodePtrs &cur_node_ptrs,
                                           ColNodePPtrs &cur_col_node_pptrs) {
  clear_node_ptrs(cur_node_ptrs);
  cur_col_node_pptrs[0][TOP].assign(cur_col_node_pptrs[0][TOP].size(), nullptr);

  for (int row = 0; row < _info._device_scale.x; ++row) {
    // actually clear LEFT, BOTTOM, RIGHT side
    for (int side = 0, count = 0; count < RRGPara::NUM_TILE_SIDES - 1; ++side) {
      if (side != TOP) {
        cur_col_node_pptrs[row][side].assign(
            cur_col_node_pptrs[row][side].size(), nullptr);
        ++count;
      }
    }

    ArchCell &tile = *cur_col_tile(row);

    // spread pptrs from above
    SideNodePPtrs &cur_side_node_pptrs = cur_col_node_pptrs[row];
    for (ArchPort *port : tile.ports())
      if (port->side() == TOP)
        spread_ptr_to_tile_port_and_grm(row, *port, cur_side_node_pptrs);

    // allocate pptrs
    for (ArchPort *tile_port : tile.ports()) {
      for (Pin *pin : tile_port->mpins()) {
        if (!cur_side_node_pptrs[tile_port->side()][pin->index()]) {
          cur_side_node_pptrs[tile_port->side()][pin->index()] =
              cur_node_ptrs.next();
        }
      }
      spread_ptr_to_tile_port_and_grm(row, *tile_port, cur_side_node_pptrs);
    }

    // copy bottom to top, except for last row
    if (row < _info._device_scale.x - 1) {
      copy(cur_col_node_pptrs[row][BOTTOM].begin(),
           cur_col_node_pptrs[row][BOTTOM].end(),
           cur_col_node_pptrs[row + 1][TOP].begin());
    }
  }
}

///// Step 3

#ifdef DEVELOPING
// slower
void RRGraph::spread_nodes_from_last_column(int cur_col,
                                            ColNodePPtrs &last_col_node_pptrs,
                                            ColNodePPtrs &cur_col_node_pptrs) {
  for (int row = 0; row < _info._device_scale.x; ++row) {
#if defined(SPREAD_FROM_LEFT_TO_RIGHT)
    Point last_col_pos(row, cur_col - 1);
    ArchCell *last_col_tile =
        FPGADesign::instance()->get_inst_by_pos(last_col_pos)->down_module();
                        for (ArchPort* tile_port: last_col_tile->ports() {
      if (tile_port->side() != RIGHT)
        continue;
      for (Pin *pin : tile_port->mpins()) {
        int idx = pin->index();
        // assert last column
        ASSERTD(last_col_node_pptrs[row][RIGHT][idx], // RRGNode**
                tile_port->name() +
                    ": pptr of tile port at last column is nullptr.");
        /*
         * actually exist tile port cell pin that connect a net which never has
         * another pin connected, even in fqvr300's arch, so release this ASSERT
         * here
         */
        //				ASSERTD(*last_col_node_pptrs[row][RIGHT][idx],
        //// RRGNode*
        ///rrg_error(tile_port.name() + ": ptr to RRGNode at last
        // column is nullptr."));
        //  assert current column
        ASSERTD(cur_col_node_pptrs[row][LEFT][idx], // RRGNode**
                tile_port->name() +
                    ": pptr of tile port at current column is nullptr.");
        if (*cur_col_node_pptrs[row][LEFT][idx]) { // RRGNode*
          ASSERTD(*cur_col_node_pptrs[row][LEFT][idx] ==
                      *last_col_node_pptrs[row][RIGHT][idx],
                  string("multi sources: ") + "No." +
                      lexical_cast<string>(idx) + "port of tile at " +
                      lexical_cast<string>(Point(row, cur_col)));
        } else {
          *cur_col_node_pptrs[row][LEFT][idx] =
              *last_col_node_pptrs[row][RIGHT][idx];
        }
      }
			}
#elif defined(SPREAD_FROM_RIGHT_TO_LEFT)
    Point last_col_pos(row, cur_col + 1);
    ArchCell *last_col_tile =
        FPGADesign::instance()->get_inst_by_pos(last_col_pos)->down_module();
    for (ArchPort *tile_port : last_col_tile->ports()) {
      if (tile_port->side() != LEFT)
        continue;
      for (Pin *pin : tile_port->mpins()) {
        int idx = pin->index();
        // assert last column
        ASSERTD(last_col_node_pptrs[row][LEFT][idx], // RRGNode**
                tile_port->name() +
                    ": pptr of tile port at last column is nullptr.");
        /*
         * actually exist tile port cell pin that connect a net which never has
         * another pin connected, even in fqvr300's arch, so release this ASSERT
         * here
         */
        //				ASSERTD(*last_col_node_pptrs[row][LEFT][idx],
        //// RRGNode* 					    tile_port.name() +
        ///": ptr to RRGNode at last column is
        // nullptr.");
        //  assert current column
        ASSERTD(cur_col_node_pptrs[row][RIGHT][idx], // RRGNode**
                tile_port->name() +
                    ": pptr of tile port at current column is nullptr.");
        if (*cur_col_node_pptrs[row][RIGHT][idx]) { // RRGNode*
          ASSERTD(*cur_col_node_pptrs[row][RIGHT][idx] ==
                      *last_col_node_pptrs[row][LEFT][idx],
                  boost::format("multi sources: No. %1% port of tile at %2%") %
                      idx % Point(row, cur_col));
        } else {
          *cur_col_node_pptrs[row][RIGHT][idx] =
              *last_col_node_pptrs[row][LEFT][idx];
        }
      }
    }
#endif
  }
}
#else
// faster
void RRGraph::spread_nodes_from_last_column(int cur_col,
                                            ColNodePPtrs &last_col_node_pptrs,
                                            ColNodePPtrs &cur_col_node_pptrs) {
  for (int row = 0; row < _info._device_scale.x; ++row) {
    for (int idx = 0, range = last_col_node_pptrs[row][RIGHT].size();
         idx < range; ++idx) {
      if (cur_col_node_pptrs[row][LEFT][idx] &&
          last_col_node_pptrs[row][RIGHT][idx])
        *cur_col_node_pptrs[row][LEFT][idx] =
            *last_col_node_pptrs[row][RIGHT][idx];
    }
  }
}
#endif

///// Step 4

static void check_net_is_inside_tile(ArchInstance &grm,
                                     RRGArchNet &net_in_grm) {
  for (Pin *grm_cell_pin : net_in_grm.pins()) {
    if (!grm_cell_pin->is_mpin())
      continue;
    RRGArchNet *net_in_tile =
        static_cast<RRGArchNet *>(grm.find_pin(grm_cell_pin->name())->net());
    ASSERT(net_in_tile, "can not find corresponding tile level net of " +
                            net_in_grm.name() + " in " +
                            net_in_grm.owner()->name());

    RRGArchNet::pin_iter tile_cell_pin_it = find_if(
        net_in_tile->pins(), [](const Pin *pin) { return pin->is_mpin(); });
    ASSERT(tile_cell_pin_it == net_in_tile->pins().end(),
           net_in_grm.name() + ": pptr of this net in grm is nullptr.");
  }
}

void RRGraph::build_grm(const Point &pos, ArchInstance &grm,
                        NodePtrs &cur_node_ptrs) {
  static const double unit_R = FPGADesign::instance()->wire_unit_resistor();
  static const double unit_C = FPGADesign::instance()->wire_unit_capacity();

  // for each net in GRM
  for (RRGArchNet *net :
       static_cast<RRGArchCell *>(grm.down_module())->nets()) {
    PtrToRRGNodePtr pptr = nullptr;
    if (!(pptr = net->ptr_to_rrg_node_ptr(pos.x))) {
#ifdef DEVELOPING
      check_net_is_inside_tile(grm, *net);
#endif
      pptr = cur_node_ptrs.next();
      net->set_ptr_to_rrg_node_ptr(pos.x, pptr);
    }

    if (*pptr == nullptr) {
      RRGNodePtr ptr = RRGFactory::instance().make_rrgnode(net);
      ptr->set_timing(unit_R, unit_C);

      *pptr = ptr;
      _nodes.add(ptr);
      _nodes_lut.at(pos)[net->index()] = ptr;
    } else {
      _nodes_lut.at(pos)[net->index()] = *pptr;
    }
    (*pptr)->update_boundary(pos);
  }

  // for each switch in GRM
  for (RRGArchInstance *elem :
       static_cast<RRGArchCell *>(grm.down_module())->instances()) {
    // need to confirm
    bool pt_flag = elem->module_type().compare(sw_types[RRGSwitch::PT]) == 0
                       ? true
                       : false;

    for (RRGArchInstance::Path *path : elem->paths()) {
      RRGArchNet *f_net = get<0>(*path);
      RRGArchNet *t_net = get<1>(*path);
      ArchPath *apath = get<2>(*path);
      RRGNode *f_node = f_net->rrg_node_ptr(pos.x);
      RRGNode *t_node = t_net->rrg_node_ptr(pos.x);

      ASSERTD(
          f_node && t_node,
          boost::format("nullptr RRGNode in path: %1% -> %2% of %3% at %4%") %
              f_net->name() % t_net->name() % elem->name() % pos);

      RRGSwitch *sw = f_node->create_switch(apath, f_node, t_node, pos);
      sw->set_from_net(f_net);
      sw->set_to_net(t_net);
      sw->set_type(elem->module_type()); // need to confirm
      ++_info._num_pips;

      if (sw->type() == RRGSwitch::BUF) { // need to confirm
        f_node->plus_Cext(sw->Cin());
        t_node->plus_Cext(sw->Cout());
      } else if (sw->type() == RRGSwitch::PT &&
                 pt_flag) { // only consider once for PT
        f_node->plus_Cext(sw->Cin());
        t_node->plus_Cext(sw->Cout());
        pt_flag = false;
      } else {
      } // dummy switch, do nothing
    }
  }
}

void RRGraph::build_grms_in_a_column(int cur_col, NodePtrs &cur_node_ptrs) {
  for (int row = 0; row < _info._device_scale.x; ++row) {
    Point cur_pos(row, cur_col);
    ArchCell &tile = *cur_col_tile(row);
    for (ArchInstance *inst : tile.instances())
      if (is_grm(inst))
        build_grm(cur_pos, *inst, cur_node_ptrs);
  }
}

} // namespace RRG
} // namespace FDU