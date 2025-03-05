#include <cstdlib> // system
#include <fstream>
// #include <boost/phoenix/core.hpp>

#include "CellGraph.h"

namespace PACK {

using namespace std;
using namespace boost;

//////////////////////////////////////////////////////////////////////////
//  constructor
GArcType::GArcType(const string &src_type, const string &tar_type, PKNet *o)
    : GObjType(str(format("%1%_ARC_%2%") % src_type % tar_type),
               GObjType::GARC),
      ori_net_(o), sink_num_type_(EQUAL) {}

GNode::GNode(CellGraph *context, const string &type, PKInstance *o)
    : GObject(context, new GNodeType(type, o)),
      node_type_(static_cast<GNodeType *>(GObject::type_ptr())) {
  if (o != nullptr)
    o->set_gobj(this);
}

GArc::GArc(CellGraph *context, GNode *source, GNode *target, PKNet *o)
    : GObject(context, new GArcType(source->type(), target->type(), o)),
      arc_type_(static_cast<GArcType *>(GObject::type_ptr())), source_(source),
      target_(target) {
  source_->add_outgoing(this);
  target_->add_incoming(this);
  o->add_gobj(this);
}

//////////////////////////////////////////////////////////////////////////
//  GNode
GNode::ComNetPairs &GNode::get_common_net(GNode *node) {
  for (CommonNet &cn : common_net_map_)
    if (cn.target == node)
      return cn.net_pairs;
  common_net_map_.push_back(CommonNet(node));
  return common_net_map_.back().net_pairs;
}

GNode::PinNamesPair *GNode::create_pname_pair(GNode *node) {
  ComNetPairs &com_net_pairs = get_common_net(node);
  com_net_pairs.push_back(PinNamesPair());
  return &com_net_pairs.back();
}

void GNode::release() {
  node_type_->ori_inst_->set_gobj(nullptr);

  vector<GArc *> arc_vec(incoming_arcs_.begin(), incoming_arcs_.end());
  for (GArc *in_arc : arc_vec) {
    in_arc->release();
    // get_context()->gobjects_.erase(*in_arc);
  }
  arc_vec.assign(outgoing_arcs_.begin(), outgoing_arcs_.end());
  for (GArc *out_arc : arc_vec) {
    out_arc->release();
    // get_context()->gobjects_.erase(*out_arc);
  }
  get_context()->type_map_[type()].remove(this);
  get_context()->gobjects_.remove(this);
}

void GNode::echo_dotfile(ofstream &o) const {
  o << "\t\"" << correspond()->name() << "\"";
  o << " [color=skyblue];\n";
}

//////////////////////////////////////////////////////////////////////////
//  GArc
void GArc::set_pin_name(PinType t, const string &name) {
  if (t == SRC_NAME)
    arc_type_->src_name_ = &name;
  else
    arc_type_->sink_names_.push_back(&name);
}

void GArc::release() {
  arc_type_->ori_net_->remove_gobj(this);

  source_->remove_outgoing(this);
  target_->remove_incoming(this);

  get_context()->type_map_[type()].remove(this);
  get_context()->gobjects_.remove(this);
}

void GArc::echo_dotfile(ofstream &o) const {
  o << "\t\"" << source_->correspond()->name() << "\""
    << " -> \"" << target_->correspond()->name() << "\"";

  string label;
  label.append("src=").append(*arc_type_->src_name_).append("\\nsink=");
  for (cstr_ptr cpstr : arc_type_->sink_names_)
    label.append(*cpstr).append(";");
  o << " [label=\"" << label << "\"];\n";
}

//////////////////////////////////////////////////////////////////////////
//  CellGraph
void CellGraph::clear() {
  type_map_.clear();
  gobjects_.clear();
}

void CellGraph::clear_layer_lock() {
  for (GObject *gobj : gobjects_)
    gobj->clear_layer_lock();
}

void CellGraph::clear_rule_lock() {
  for (GObject *gobj : gobjects_)
    gobj->clear_rule_lock();
}

void CellGraph::build_graph() {
  clear();
  context_->parse_nets();

  GNode *gnode;
  for (PKInstance *inst : context_->instances()) {
    gnode = create_gnode(this, inst->down_module()->name(), inst);

    if (context_->type() == KEY_WORD::RULE_CELL) {
      RuleInstance *rinst = static_cast<RuleInstance *>(inst);
      gnode->set_rule_to_lock(
          rinst->rule_lock()); // whether the rule_lock_ attribute of
                               // ruleInstance is true or false
    }
  }

  GArc *garc;
  for (PKNet *net : context_->nets()) {
    // cout<<"Net name: "<<net.name()<<"\nSink pins: "<<net.sink_pins()<<"\nSrc
    // pins: "<<net.source_pins()<<endl;
    for (const SimpleNet &snet : net->simple_nets()) {
      PKPin *src_pin = snet.src;

      if (src_pin->is_mpin())
        continue; // nets connecting to cell pins are all neglected

      garc = create_garc(this, src_pin->instance()->gobject(),
                         snet.inst->gobject(), net);

      // garc->set_sink_num(net.num_pure_sinks());
      garc->set_pin_name(GArc::SRC_NAME, src_pin->name());
      for (PKPin *pin : snet.sinks)
        garc->set_pin_name(GArc::SINK_NAME, pin->name());
    }
  }

  if (context_->type() == KEY_WORD::RULE_CELL)
    build_rule_cell();
  // draw();
}

void CellGraph::build_rule_cell() {
  RuleCell *rule_cell = static_cast<RuleCell *>(context_);

  for (RulePort *port : rule_cell->ports()) { // todo: pins
    PKNet *net = static_cast<PKNet *>(port->mpin()->net());

    if (port->dir() == COS::OUTPUT) {
      GArcType::SinkNumType sink_num_type =
          port->is_connect() ? GArcType::MORE : GArcType::NOT_LESS;
      for (GArc *arc : net->gobjects())
        arc->set_sink_num_type(sink_num_type);
    } else {
      const SimpleNet *prev_snet = nullptr;
      GNode *prev_node = nullptr;
      for (const SimpleNet &snet : net->simple_nets()) {
        PKPin *src_pin = snet.src;
        //					PKPin* asink_pin =
        // snet.second[0];
        GNode *node = snet.inst->gobject();

        // prevent multi sources
        if (src_pin != static_cast<PKPin *>(port->mpin()))
          continue;

        // add common net information of GNode
        GNode::PinNamesPair *ppair_of_prev_node = nullptr;
        if (prev_node != nullptr)
          ppair_of_prev_node = prev_node->create_pname_pair(node);

        // add connect pin for present node and add common net info for previous
        // node
        for (PKPin *pin : snet.sinks) {
          if (port->is_connect())
            node->add_connect_pin(&pin->name());
          if (ppair_of_prev_node != nullptr)
            ppair_of_prev_node->second.push_back(&pin->name());
        }

        // if previous node is not nullptr, add common net info for present node
        if (prev_node != nullptr) {
          for (PKPin *pin : prev_snet->sinks)
            ppair_of_prev_node->first.push_back(&pin->name());
        }

        prev_node = node;
        prev_snet = &snet;
      }
    }
  }

  for (PKNet *net : rule_cell->nets()) {
    for (PKPin *src_pin : net->source_pins()) {
      if (!src_pin->is_mpin()) {
        GNode *node = src_pin->instance()->gobject();

        bool outignore = false;
        for (PKPin *sink_pin : net->sink_pins())
          if (sink_pin->is_mpin() &&
              static_cast<RulePort *>(sink_pin->port())->is_bus_ignore())
            outignore = true;
          else
            outignore = false; // if out-port connect to another internal pin,
                               // default ignore is invalid
        if (!outignore)
          node->add_connect_pin(&src_pin->name());

        for (PKPin *sink_pin : net->sink_pins())
          if (!sink_pin->is_mpin())
            sink_pin->instance()->gobject()->add_connect_pin(&sink_pin->name());
        break;
      }
    }
  }
}

void CellGraph::fill_type_map() {
  if (type_map_.size() != 0)
    return;
  for (GObject *obj : gobjects_)
    type_map_[obj->type()].push_back(obj);
}

CellGraph::GObjList *CellGraph::find_gobjs(const string &n) {
  TypeGObjMap::iterator it = type_map_.find(n);
  if (it == type_map_.end())
    return nullptr;
  else
    return &(it->second);
}

// rebuild graph incrementally
void CellGraph::after_inst_created(PKInstance *inst) {
  ASSERTD(inst->gobject() == nullptr, "gnode already created");
  create_gnode(this, inst->down_module()->name(), inst);
}

void CellGraph::before_pin_unhooked(PKNet *net, PKPin *pin) {
  if (pin->is_mpin() || net == nullptr)
    return;

  vector<GArc *> all_arcs;
  if (pin->dir() == COS::OUTPUT) {
    all_arcs.assign(net->gobjects().begin(), net->gobjects().end());
    PKInstance *src_inst = pin->instance();
    for (GArc *garc : all_arcs) {
      if (garc->source()->correspond() == src_inst)
        garc->release();
    }
  } else if (pin->dir() == COS::INPUT) {
    net->dec_pure_sinks();

    all_arcs.assign(net->gobjects().begin(), net->gobjects().end());
    PKInstance *sink_inst = pin->instance();
    for (GArc *garc : all_arcs) {
      if (garc->target()->correspond() == sink_inst) {
        if (garc->num_sinks() == 1)
          garc->release();
        else
          garc->remove_sink_pname(&pin->name());
      }
    }
  }
}

void CellGraph::before_pin_hookuped(PKNet *net, PKPin *pin) {
  if (pin->is_mpin() || net == nullptr)
    return;

  if (pin->dir() == COS::OUTPUT && net->num_pure_sinks() != 0) {
    map<PKInstance *, GArc *> cps_inst_map;
    for (PKPin *spin : net->sink_pins()) {
      if (spin->is_mpin())
        continue;
      PKInstance *sinst = spin->instance();
      GArc *garc = cps_inst_map[sinst];
      if (!garc) {
        // build arc for new output pin
        garc = cps_inst_map[sinst] = create_garc(
            this, pin->instance()->gobject(), sinst->gobject(), net);
        garc->set_pin_name(GArc::SRC_NAME, pin->name());
      }
      garc->set_pin_name(GArc::SINK_NAME, spin->name());
    }
  } else if (pin->dir() == COS::INPUT) {
    net->inc_pure_sinks();

    bool inst_found = false;
    for (GArc *garc : net->gobjects()) {
      PKInstance *sink_inst = pin->instance();
      if (garc->target()->correspond() == pin->instance()) {
        garc->add_sink_pname(&pin->name());
        inst_found = true;
      }
    }
    if (!inst_found) {
      for (PKPin *src_pin : net->source_pins()) {
        if (src_pin->is_mpin())
          continue;

        GArc *garc = create_garc(this, src_pin->instance()->gobject(),
                                 pin->instance()->gobject(), net);

        garc->set_pin_name(GArc::SRC_NAME, src_pin->name());
        garc->set_pin_name(GArc::SINK_NAME, pin->name());
      }
    }
  }
}

void CellGraph::before_pin_rehook(PKNet *old_net, PKNet *new_net, PKPin *pin) {
  before_pin_unhooked(old_net, pin);
  before_pin_hookuped(new_net, pin);
}

void CellGraph::before_inst_remove(PKInstance *inst) {
  if (inst->gobject() == nullptr)
    return;
  inst->gobject()->release();
}

void CellGraph::before_net_remove(PKNet *net) {
  vector<GArc *> all_arcs(net->gobjects().begin(), net->gobjects().end());
  for (GArc *garc : all_arcs) {
    garc->release();
  }
}

// debug interface
void CellGraph::draw() {
  ofstream out("temp.dot");

  out << "digraph \"" << context_->name() << "\"{\n";
  out << "\tnode [style=filled]\n";

  for (GObject *gobj : gobjects_)
    gobj->echo_dotfile(out);

  out << "}";

  string cmd = str(format("dot -Tpng temp.dot -o cell_graph\\%1%%2%.png") %
                   context_->name() % output_graph_idx_++);

  out.close();
  std::system(cmd.c_str());
  std::system("del temp.dot");
}
} // namespace PACK