#ifndef ROUTE_DELAY_HPP
#define ROUTE_DELAY_HPP

#include "net_walk_guider.hpp"
#include "rd_tree.hpp"
#include "rrg/rrg.hpp"
#include "rtnetlist.hpp"
#include "sta_arg.hpp"

using namespace FDU::RRG;
using namespace FDU;
using namespace COS;

namespace FDU {
namespace STA {

//! route delay engine
class RDEngine {

public:
  // typedef std::map<RRGNode*, Pin*>       node_pin_map;
  // typedef node_pin_map::value_type       node_pin_pair;

  typedef std::map<std::string, RDTree *> tree_map;

  typedef std::vector<TilePin *> TilePins;

  typedef std::vector<RRGSwitch *> Switches;
  typedef std::map<Point, Switches> switch_repo;

  RDEngine(TDesign *target, STAArg *arg) : target_(target), arg_(arg) {}
  virtual ~RDEngine() {}

  void compute_route_delay();

protected:
  void initial_tools();

  string line_node_name(const Point &from, const Point &to) const {
    return (boost::format(cur_tree_->line_node_name_rule()) %
            cur_seg_->full_name() % from % to)
        .str();
  }

  string switch_node_name(RRGSwitch *sw, RRGNode *seg) const {
    return (boost::format(cur_tree_->switch_node_name_rule()) %
            seg->full_name() % sw->from_net() % sw->to_net() % sw->pos())
        .str();
  }

  void process_net(COSRTNet *net);
  void build_net_tree(COSRTNet *net);

  void check_switch_attach(const Point &pos, RDNode *driver);

  void create_src_segment_nodes();
  void create_normal_segment_nodes(RDNode *driver, RRGSwitch *rrg_sw);

  void set_seg(RRGNode *seg);

  void stretch_tile_pin_nodes(RDNode *driver, const string &tpin_name,
                              ArchInstance *inst);
  void stretch_pins_to_adjacent_tiles(const TilePins &tpins, RDNode *driver,
                                      const Point &driver_pos);

  TDesign *target_;

  NetWalkGuider walk_guider_;
  RRGraph rrg_;
  // tree_map           net_trees_;

  RDTreePtr cur_tree_;
  RRGNode *cur_seg_;
  switch_repo cur_switch_repo_;

  STAArg *arg_;
};

} // namespace STA
} // namespace FDU

#endif
