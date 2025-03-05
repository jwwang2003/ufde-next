#include "rd_engine.hpp"
#include "sta_utils.hpp"

using namespace std;
using namespace boost;

#include <queue>

namespace FDU {
namespace STA {

static Property<Point> &POS = create_property<Point>(INSTANCE, "position");

void RDEngine::initial_tools() {
  FDU_LOG(INFO) << (progress_fmt % 40 % "building rrg").str();
  rrg_.build_rrg();
  FDU_LOG(INFO) << (progress_fmt % 50 % "building net walk guider").str();
  walk_guider_.build();
}

void RDEngine::compute_route_delay() {
  initial_tools();
  for (Net *net : target_->top_module()->nets())
    process_net(static_cast<COSRTNet *>(net));
}

void RDEngine::process_net(COSRTNet *net) {
  if (!net || !net->num_pips())
    return; // check net

  build_net_tree(net);
  cur_tree_->post_build_process();

  string new_net_name = net->name();
  for (int i = 0; i < new_net_name.size(); ++i)
    if (new_net_name[i] == '/')
      new_net_name[i] = '-'; // change to a valid file name prefix

  if (arg_->dump_spice_for_net)
    cur_tree_->dump_spice_netlist(new_net_name);
  if (arg_->dump_dot_for_net)
    cur_tree_->dump_dot_file(new_net_name);

  // for debug
  /*
   *if (cur_tree_->owner()->name().find("1988") != string::npos)
   *{
   *    cur_tree_->root()->recursive_print_cap();
   *    cur_tree_->root()->recursive_print_timing();
   *}
   */
}

void RDEngine::build_net_tree(COSRTNet *net) {
  //! 1. initial net tree
  FDU_LOG(INFO) << "processing net: " << net->name();
  cur_tree_ = RDFactory::instance().make_tree(net);
  cur_tree_->initial(rrg_);

  //! 2. build source segment nodes
  create_src_segment_nodes();

  //! 3. build normal & sink sgement nodes
  for (const RDTree::Pip &pip : cur_tree_->pips()) {
    RRGSwitch *sw = pip.get<1>();
    RRGNode *from = pip.get<0>();
    RRGNode *to = pip.get<2>();

    string node_name = switch_node_name(sw, from);
    RDNode *driver = cur_tree_->find_node_by_name(node_name);
    if (driver == nullptr) {
      // FDU_LOG(WARN) << "cannot find driver" << " select one:";
      int i = 0;
      RDNode *root = cur_tree_->root();
      std::queue<RDNode *> nodes;
      nodes.push(root);
      while (!nodes.empty()) {
        root = nodes.front();
        nodes.pop();
        // FDU_LOG(DEBUG) << "node: " << root->name() << endl;
        for (RDNode *child : root->children()) {
          nodes.push(child);
        }

        if (root->num_children() == 0 && nodes.empty()) {
          driver = root;
          FDU_LOG(INFO) << "select: " << driver->name() << " as driver";
          break;
        }
      }

    } else {
      FDU_LOG(DEBUG) << "select: " << driver->name() << " as driver";
    }
    ASSERT(driver, "cannot find switch driver: " + node_name);
    FDU_LOG(DEBUG) << "find driver: " << node_name;
    driver->open(); // open this switch

    set_seg(to);
    create_normal_segment_nodes(driver, sw);

    // check sink pin node creation
    if (Pin *sink_pin = cur_tree_->find_sink_pin_by_seg(to)) {
      Point sink_pos = sink_pin->instance()->property_value(POS);

      // must be at the same position
      // ASSERT(sink_pos.equal2d(sw->pos()), "unsupport sink type");
      if (!sink_pos.equal2d(sw->pos())) {
        FDU_LOG(WARN) << "sink pin pos not equal to last switch pos";
      }

      // connect output to switch node
      RDNode *onode =
          cur_tree_->add_node(sink_pin->path_name(), RDNode::OUTPUT, driver, 0);
      cur_tree_->add_snk_node_pin(onode, sink_pin);
    }
  }
}

void RDEngine::set_seg(RRGNode *seg) {
  cur_seg_ = seg;
  FDU_LOG(DEBUG) << "set seg to: " << seg->full_name();
  cur_switch_repo_.clear();
  for (RRGSwitch *sw : seg->switches())
    cur_switch_repo_[sw->pos()].push_back(sw);

  for (switch_repo::iterator it = cur_switch_repo_.begin();
       it != cur_switch_repo_.end(); ++it) {
    FDU_LOG(DEBUG) << "\tswitch pos: " << it->first << endl;
  }
}

void RDEngine::stretch_pins_to_adjacent_tiles(const TilePins &tpins,
                                              RDNode *driver,
                                              const Point &driver_pos) {
  // check switch nodes
  check_switch_attach(driver_pos, driver);

  // stretch all tile pins
  for (const TilePin *tpin : tpins) {
    Point to_pos(tpin->offset_point() + driver_pos);
    Point scale = FPGADesign::instance()->scale();
    if (to_pos.x < 0 || to_pos.x >= scale.x || to_pos.y < 0 ||
        to_pos.y >= scale.y)
      continue; // skip illegal pos

    // check arch inst
    ArchInstance *to_inst = FPGADesign::instance()->get_inst_by_pos(to_pos);
    ASSERT(to_inst,
           "cannot find arch instance: " + lexical_cast<string>(to_pos));

    RDNode *line_node = cur_tree_->add_node(line_node_name(driver_pos, to_pos),
                                            RDNode::LINE, driver);
    ASSERT(line_node, "add line node error");

    // stretch node
    stretch_tile_pin_nodes(line_node, tpin->reverse_name(), to_inst);
  }
}

void RDEngine::stretch_tile_pin_nodes(RDNode *driver, const string &tpin_name,
                                      ArchInstance *inst) {
  TilePins tpins =
      walk_guider_.find_tile_pin_pins(inst->down_module()->name(), tpin_name);
  stretch_pins_to_adjacent_tiles(tpins, driver, inst->logic_pos());
}

void RDEngine::check_switch_attach(const Point &pos, RDNode *driver) {
  switch_repo::iterator it = cur_switch_repo_.find(pos);
  if (it == cur_switch_repo_.end())
    return;

  // line type or input type
  // check switch & attach all
  if (driver->type() == RDNode::LINE || driver->type() == RDNode::INPUT) {
    for (RRGSwitch *rrg_sw : it->second) {
      string node_name = switch_node_name(rrg_sw, cur_seg_);
      RDNode *sw_node =
          cur_tree_->add_node(node_name, RDNode::SWITCH, driver, rrg_sw);
      ASSERTD(sw_node, "add switch node error");
    }
    cur_switch_repo_.erase(it);
  }

  // switch
  else if (driver->type() == RDNode::SWITCH ||
           driver->type() == RDNode::DUMMY) // normal switch or dummy switch
  {
    RRGSwitch *next_sw = 0;
    // find the next switch driver driven by now seg
    for (RRGSwitch *rrg_sw : it->second)
      if (rrg_sw->from_node() == cur_seg_) {
        next_sw = rrg_sw;
        break;
      }

    // normal this cituation won't happen
    // however, when a segment has no length it will happen
    if (next_sw) {
      // connect driver to next driver
      RDNode *next_driver = cur_tree_->add_node(
          switch_node_name(next_sw, cur_seg_), RDNode::SWITCH, driver, next_sw);

      for (RRGSwitch *rrg_sw : it->second)
        if (rrg_sw != next_sw) {
          // connect all other switches to next driver
          cur_tree_->add_node(switch_node_name(rrg_sw, cur_seg_),
                              RDNode::SWITCH, next_driver, rrg_sw);
        }
    }
  }
}

void RDEngine::create_src_segment_nodes() {
  set_seg(cur_tree_->pips().begin()->get<0>());

  // check pos condition
  RRGSwitch *rrg_sw = cur_tree_->pips().begin()->get<1>();
  Point src_pin_pos = cur_tree_->driver_pin()->instance()->property_value(POS);
  ASSERT(src_pin_pos.equal2d(rrg_sw->pos()), "unsupport source type");

  ArchInstance *inst = FPGADesign::instance()->get_inst_by_pos(rrg_sw->pos());
  RDNode *driver = cur_tree_->add_node(cur_tree_->driver_pin()->path_name(),
                                       RDNode::INPUT, 0);
  cur_tree_->set_src_node_pin(driver, cur_tree_->driver_pin());

  // note: using from_net of RRGSwitch
  TilePins tpins = walk_guider_.find_grm_net_pins(inst->down_module()->name(),
                                                  rrg_sw->from_net()->name());
  stretch_pins_to_adjacent_tiles(tpins, driver, rrg_sw->pos());
}

void RDEngine::create_normal_segment_nodes(RDNode *driver, RRGSwitch *rrg_sw) {
  ArchInstance *inst = FPGADesign::instance()->get_inst_by_pos(rrg_sw->pos());
  TilePins tpins = walk_guider_.find_grm_net_pins(inst->down_module()->name(),
                                                  rrg_sw->to_net()->name());
  stretch_pins_to_adjacent_tiles(tpins, driver, rrg_sw->pos());
}

} // namespace STA
} // namespace FDU
