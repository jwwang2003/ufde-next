#include "RTFactory.h"
#include "RTCstLoadHandler.h"
#include "RTMsg.h"
#include "RTNode.h"
#include "arch/archlib.hpp"
#include "rutils.h"
#include <iostream>
#include <math.h>

namespace FDU {
namespace RT {

using namespace std;
using namespace ARCH;

#define using_ise_plc

void RTNet::init_rt_info(RTGraph *rrg, int bb_fac, const CstNets *_pCstNets) {
  if (_pCstNets) {
    // if cstFile contains this net.
    const CstNet *cstNet = _pCstNets->find_net(this->name());
    if (cstNet && cstNet->num_pips() > 0) {
      ASSERT(num_pips() == 0,
             this->name() + ": you have pips infomation in input netlist file,\
					and also in router constraint file, please just remain one of them, or none of them...");
      for (const PIP *pip : cstNet->routing_pips()) {
        this->create_pip(pip->from(), pip->to(), pip->position(), pip->dir());
      }
    }
  }

  // partial routed, either from input netlist file or cst file.
  if (num_pips() > 0) {
    load_src_sink_nodes(rrg);
    load_info_for_routed_net(rrg);
    if (is_completely_routed(rrg)) {
      is_ignore_ = true;
      return;
    } else {
      is_partial_routed_ = true;
    }
  }
  for (const Pin *pin : this->pins())
    if (pin->is_mpin()) {
      is_ignore_ = true;
      return;
    }

#ifdef using_ise_plc
  if (pins().size() < 2) {
    is_ignore_ = true;
    return;
  }

  for (const Pin *pin : this->pins()) {
    if (pin->name().find("CLK") != string::npos ||
        pin->name().find("CK") != string::npos) {
      set_type(NetType::CLOCK);
      break;
    } else if (pin->name().find("SR") != string::npos ||
               pin->name().find("RST") != string::npos) {
      set_type(NetType::RESET);
      break;
    } else if (pin->instance()->module_type() == "TBUF") {
      set_type(NetType::TBUF);
      break;
    }
  }
#endif
  load_bb_fac(bb_fac);
  if (!is_partial_routed_) {
    load_src_sink_nodes(rrg);
  }
}

void RTNet::load_bb_fac(int bb_fac) {
  // set spread range(left_top, right_bottom) for a net
  FPGADesign *p_fpga_design = FPGADesign::instance();
  int nx = p_fpga_design->scale().x, ny = p_fpga_design->scale().y;

  bb_.bblt.x = nx;
  bb_.bblt.y = ny;
  bb_.bbrb.x = 0;
  bb_.bbrb.y = 0;

  if (type() == CLOCK || type() == RESET || type() == TBUF) {
    bb_.bblt.x = 0;
    bb_.bblt.y = 0;
    bb_.bbrb.x = nx;
    bb_.bbrb.y = ny;
    return;
  }

  Property<Point> &postion =
      create_property<Point>(COS::INSTANCE, RT_CONST::POSITION);

  for (Pin *pin : pins()) {
    Point pos = pin->instance()->property_value(postion);
    ASSERT(pos.x >= 0 && pos.y >= 0 && pos.z >= 0 && pos.x <= nx && pos.y <= ny,
           ErrMsg(ErrMsg::RTERR_INST_UPLC) % pin->instance()->name());
    if (pos.x < bb_.bblt.x)
      bb_.bblt.x = pos.x;
    if (pos.x > bb_.bbrb.x)
      bb_.bbrb.x = pos.x;

    if (pos.y < bb_.bblt.y)
      bb_.bblt.y = pos.y;
    if (pos.y > bb_.bbrb.y)
      bb_.bbrb.y = pos.y;
  }
  bb_.bblt.x = max(bb_.bblt.x - bb_fac, 0);
  bb_.bblt.y = max(bb_.bblt.y - bb_fac, 0);
  bb_.bbrb.x = min(bb_.bbrb.x + bb_fac, nx - 1);
  bb_.bbrb.y = min(bb_.bbrb.y + bb_fac, ny - 1);
}

void RTNet::load_src_sink_nodes(RTGraph *rrg) {
  Property<Point> &postion =
      create_property<Point>(COS::INSTANCE, RT_CONST::POSITION);

  Pin *src_pin = *(source_pins().begin());
  Point pos = src_pin->instance()->property_value(postion);
  ASSERT(pos.x != -1 && pos.y != -1,
         ErrMsg(ErrMsg::RTERR_POS) % "source pin" % src_pin->name());

  // load source node
  src_node_ = static_cast<RTNode *>(
      rrg->find_logic_pin_node(src_pin->instance(), *src_pin, pos));
  ASSERT(src_node_ != nullptr,
         ErrMsg(ErrMsg::RTERR_UFND_NODE) % "source" % src_pin->name() % name());
  src_node_->set_src();

  // load sink nodes
  for (Pin *sink_pin : sink_pins()) {
    pos = sink_pin->instance()->property_value(postion);
    ASSERT(pos.x != -1 && pos.y != -1,
           ErrMsg(ErrMsg::RTERR_POS) % "sink pin" % sink_pin->name());
    RTNode *sink_node = static_cast<RTNode *>(
        rrg->find_logic_pin_node(sink_pin->instance(), *sink_pin, pos));
    ASSERT(sink_node != nullptr, ErrMsg(ErrMsg::RTERR_UFND_NODE) % "sink" %
                                  sink_pin->name() % name());
    sink_node->set_sink();
    sink_nodes_.push_back(sink_node);
  }
}

void RTNet::mark_sinks() {
  vector<RTNode *>::iterator node_it = sink_nodes_.begin(),
                             end_node = sink_nodes_.end();
  for (; node_it != end_node; ++node_it)
    (*node_it)->inc_tarflg();
}

void RTNet::load_info_for_routed_net(RTGraph *rrg) {
  bool next_pip_from_net_record = false;
  for (const PIP *pip : routing_pips()) {
    if (pip == routing_pips().front() || next_pip_from_net_record) {
      next_pip_from_net_record = false;
      RTNode *from_node = static_cast<RTNode *>(
          rrg->find_grm_net_node(pip->from(), pip->position()));
      ASSERT(from_node,
             "can not find pip, may be you are using a different arch file...");
      from_node->add_occ();
      fixed_path_.push_back(from_node);
    }
    RTNode *to_node = static_cast<RTNode *>(
        rrg->find_grm_net_node(pip->to(), pip->position()));
    ASSERT(to_node,
           "can not find pip, may be you are using a different arch file...");
    to_node->add_occ();
    fixed_path_.push_back(to_node);
    if (to_node->is_sink()) {
      to_node->set_has_found();
      next_pip_from_net_record = true;
    }
    if (pip == routing_pips().back()) {
      ASSERT(to_node->is_sink(), ErrMsg(ErrMsg::NLTERR_LAST_PIP_ILLEGAL) %
                                     pip->to() % this->name());
    }
  }
  seg_begin_ = fixed_path_.begin();
  seg_end_ = fixed_path_.end();
  --seg_end_;
}

void RTNet::load_info_from_fixed_path() {
  path_.assign(fixed_path_.begin(), fixed_path_.end());
  for (RTNode *node : path_) {
    node->dec_occ();
    node->set_pathnode(true);
  }
  seg_begin_ = path_.begin();
  seg_end_ = path_.end();
  --seg_end_;
}

bool RTNet::is_completely_routed(RTGraph *rrg) {
  for (const RTNode *sink_node : sink_nodes()) {
    if (!sink_node->get_has_found()) {
      return false;
    }
  }
  return true;
}

bool RTNet::is_feasible() {
  int unf_num = 0;
  path_node_iter node_it = path_.begin(), end_node = path_.end();
  for (; node_it != end_node; ++node_it) {
    if ((*node_it)->occ() > (*node_it)->capacity()) {
      ++unf_num;
      return false;
    }
  }

  if (unf_num)
    return false;
  else
    return true;
}

void RTNet::path_update_one_cost(path_node_iter path_str, int add_or_sub,
                                 double pres_fac) {
  path_node_iter node_it = path_str, end_node = path_.end();
  while (node_it != end_node) {
    int occ = (*node_it)->occ() + add_or_sub;
    (*node_it)->set_occ(occ);
    (*node_it)->update_pres_cost(pres_fac);

    if ((*node_it)->is_sink()) {
      ++node_it;
      if (node_it == end_node)
        break;
    }
    ++node_it;
  }
}

RTNet::path_node_iter RTNet::update_trace_back(RTNode *sink_node) {
  path_.push_back(sink_node);
  sink_node->set_pathnode(true);
  path_node_iter temptail = path_.end();
  --temptail;

  RTNode *node_ptr = sink_node;
  RTNode *prev_node_ptr;
  while (node_ptr != nullptr) {
    prev_node_ptr = node_ptr->prev_node();
    if (prev_node_ptr != nullptr) {
      prev_node_ptr->set_pathnode(true);
      temptail = path_.insert(temptail, prev_node_ptr);
    }
    node_ptr = prev_node_ptr;
  }

  if (path_.size() == 0) {
    FDU_LOG(ERR) << "path(" << name() << ") empty......\n";
  }

  path_node_iter ret_iter;
  if (seg_end_ != path_.end()) {
    // for the other sink nodes of the net
    // return ret_iter from the second node of current branch path, for the
    // branch path must have one common node with trunk path
    temptail = seg_end_;
    ++temptail;
    ++temptail;
    ret_iter = temptail;
    temptail = path_.end();
    --temptail;
    seg_end_ = temptail;
  } else {
    // for the first sink node of the net
    seg_begin_ = path_.begin();
    ret_iter = seg_begin_;
    temptail = path_.end();
    --temptail;
    seg_end_ = temptail;
  }

  return ret_iter;
}

void RTNet::save_path() {
  if (path_.size() == 0)
    return;
  if (is_partial_routed()) {
    clear_pips();
  }
  RRGSwitch *p_sw;
  path_node_iter from_node_it, to_node_it;
  path_node_iter end_node = path_.end();
  --end_node; // once two nodes;

  for (from_node_it = path_.begin(); from_node_it != end_node; ++from_node_it) {
    if ((*from_node_it)->is_sink())
      continue;
    to_node_it = from_node_it;
    ++to_node_it;

    p_sw = (*from_node_it)->find_switch(*to_node_it);
    ASSERT(p_sw != nullptr, ErrMsg(ErrMsg::RTERR_UFND_SWH) %
                             (*from_node_it)->full_name() %
                             (*to_node_it)->full_name());
    create_pip(p_sw->from_net()->name(), p_sw->to_net()->name(), p_sw->pos());
  }
}
} // namespace RT
} // namespace FDU