#include <boost/range/adaptors.hpp>
#include <tuple> // tie

#include "tengine.hpp"
#include "tnetlist.hpp"
// #include "PropertyName.h"

#define ALLOW_DANGLING

#define filtered_foreach(type, var, rng, cond)                                 \
  for (type var :                                                              \
       rng | boost::adaptors::filtered([](type p) { return p->cond; }))
#define filtered_foreach2(type, var, rng, cond1, cond2)                        \
  for (type var : rng | boost::adaptors::filtered(                             \
                            [](type p) { return p->cond1 && p->cond2; }))
#define reversed_foreach(var, rng) for (var : rng | boost::adaptors::reversed)

namespace COS {

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////
//  TFactory

TFactory::Pointer TFactory::_instance(new TFactory());
TNode *TFactory::make_tnode(TNode::TNodeType type, Pin *owner, int index) {
  return new TNode(type, owner, index);
}
TEdge *TFactory::make_tedge(TNode *f_tnode, TNode *t_tnode, double r_delay,
                            double f_delay) {
  return new TEdge(f_tnode, t_tnode, r_delay, f_delay);
}

///////////////////////////////////////////////////////////////////////////////////////
//  Properties
Property<TNode *> TNODE;
Property<TData> &DELAY = create_temp_property<TData>(PIN, "delay");
Property<double> &SLACK = create_temp_property<double>(PIN, "slack");
Property<pair<TNode *, TNode *>> SEQ_TNODES;

///////////////////////////////////////////////////////////////////////////////////////
//  TNode

TEdge *TNode::create_out_edge(TNode *t_tnode,
                              double r_delay /* = TPara::ZERO */,
                              double f_delay /* = TPara::ZERO */) {
  return t_tnode->_in_tedges.add(_out_tedges.add(
      TFactory::instance().make_tedge(this, t_tnode, r_delay, f_delay)));
}

void TNode::update_pinput_tarrs(int t_index) {
  _t_arr_vec[t_index].set_value(TPara::ZERO, TPara::ZERO);
}

void TNode::update_poutput_treqs(const IntTDataMap &max_delays) {
  for (const IntTDataPair &delay : max_delays)
    _t_req_vec[delay.first].set_value(delay.second._rising,
                                      delay.second._falling);
}

void TNode::update_normal_tarrs(const TEdge *f_tedge) {
  tdata_iter tarr_it = t_arrs().begin();
  for (const TData &tdata : f_tedge->from_node()->t_arrs()) {
    tarr_it->_rising =
        max(tarr_it->_rising, tdata._rising + f_tedge->r_delay());
    tarr_it->_falling =
        max(tarr_it->_falling, tdata._falling + f_tedge->f_delay());
    ++tarr_it;
  }
}

void TNode::update_normal_treqs(const TEdge *t_tedge) {
  tdata_iter treq_it = t_reqs().begin();
  for (const TData &tdata : t_tedge->to_node()->t_reqs()) {
    treq_it->_rising =
        min(treq_it->_rising, tdata._rising - t_tedge->r_delay());
    treq_it->_falling =
        min(treq_it->_falling, tdata._falling - t_tedge->f_delay());
    ++treq_it;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//  TEngine

double TEngine::timing_analyse(WORK_MODE mode) {
  if (mode == REBUILD)
    create_tgraph();
  else { // INCREMENT
    ASSERT(_is_built, "tgraph: not built.");
    load_rt_delay();
  }
  compute_tgraph();

  return average_max_delay();
}

void TEngine::create_tgraph() {
  _tnodes.clear();
  create_all_nodes();
  create_all_conns();
  topo_sort();
  _is_built = true;
}

void TEngine::compute_tgraph() {
  initial_tdata();
  compute_tarr();
  compute_treq();
  compute_slack();
}

TNode *TEngine::create_node(TNode::TNodeType type, Pin *owner,
                            bool set_pin_prop /* = true */) {
  TNode *tnode = nullptr;
  if (!(tnode = owner->property_value(TNODE))) {
    tnode =
        _tnodes.add(TFactory::instance().make_tnode(type, owner, num_tnodes()));
    if (set_pin_prop)
      owner->set_property(TNODE, tnode);
  }
  return tnode;
}

void TEngine::create_module_pin_node(Pin *mpin) {
  //		ASSERTD(port_pin.is_connected(), tengine_error("dangling: top
  // module port."));
  if (!mpin->is_connected())
    return;

  Pin *pad_pin = mpin->is_sink() ? *mpin->net()->source_pins().begin()
                                 : *mpin->net()->sink_pins().begin();

  TNode *port_node =
      create_node(mpin->is_sink() ? TNode::OUT_PORT : TNode::IN_PORT, mpin);
  TNode *pad_node = create_node(
      mpin->is_sink() ? TNode::CELL_OPIN : TNode::CELL_IPIN, pad_pin);

  if (mpin->is_source())
    port_node->create_out_edge(pad_node);
}

void TEngine::create_composite_inst_pin_node(Pin *pin) {
  if (!pin->is_connected())
    return;

  TNode *node =
      create_node(pin->is_sink() ? TNode::CELL_IPIN : TNode::CELL_OPIN, pin);
  pin->down_pin()->set_property(TNODE, node);
}

void TEngine::create_primitive_inst_pin_node(Pin *pin) {
  bool is_clk_pin = pin->type() == CLOCK;
  if (!pin->is_connected()) {
#ifdef ALLOW_DANGLING
#else
    ASSERT(!is_clk_pin, tengine_error("dangling: clock pin dangling."));
#endif
    return;
  }

  if (pin->is_sink()) {
    create_node(is_clk_pin ? TNode::CELL_CLKPIN : TNode::CELL_IPIN, pin);
    if (is_clk_pin) {
      TNode *seq_src = _tnodes.add(
          TFactory::instance().make_tnode(TNode::SEQ_SRC, pin, num_tnodes()));
      TNode *seq_sink = _tnodes.add(
          TFactory::instance().make_tnode(TNode::SEQ_SINK, pin, num_tnodes()));
      pin->instance()->set_property(SEQ_TNODES, make_pair(seq_src, seq_sink));
    }
  } else if (pin->is_source()) {
    TNode *opin_node = create_node(TNode::CELL_OPIN, pin);
    // constant generator
    if (!pin->instance()->has_input()) {
      TNode *const_gen_src = _tnodes.add(TFactory::instance().make_tnode(
          TNode::CONST_GEN_SRC, pin, num_tnodes()));
      const_gen_src->create_out_edge(opin_node, TPara::T_CONST_GEN,
                                     TPara::T_CONST_GEN);
    }
  }
}

void TEngine::create_inst_nodes(Instance *inst) {
  if (inst->is_composite()) {
    for (Pin *pin : inst->pins())
      create_composite_inst_pin_node(pin);
    for (Instance *inst : inst->down_module()->instances())
      create_inst_nodes(inst);
  } else
    for (Pin *pin : inst->pins())
      create_primitive_inst_pin_node(pin);
}

void TEngine::create_all_nodes() {
  Module *top_module = _target->top_module();

  for (Pin *mpin : top_module->pins())
    create_module_pin_node(mpin);
  for (Instance *inst : top_module->instances())
    create_inst_nodes(inst);
}

void TEngine::create_conn(Pin *src, Pin *sink) {
#ifdef ALLOW_DANGLING
  TNode *src_node = src->property_value(TNODE);
  TNode *sink_node = sink->property_value(TNODE);
#else
  TNode *src_node = src->property_value(TNODE);
  TNode *sink_node = sink->property_value(TNODE);
#endif
  if (src_node && sink_node)
    src_node->create_out_edge(sink_node);
}

void TEngine::create_composite_inst_conns(Instance *inst) {
  filtered_foreach(Pin *, pin, inst->pins(), is_connected()) {
    if (pin->is_source())
      for (Pin *sink : pin->net()->sink_pins())
        create_conn(pin, sink);
    else if (pin->is_sink()) {
      Net *net = pin->down_pin()->net();
      ASSERT(net, "dangling: dangling module pin.");
      for (Pin *sink : net->sink_pins())
        create_conn(pin, sink);
    }
  }
  for (Instance *inst : inst->down_module()->instances())
    create_inst_conns(inst);
}

void TEngine::create_primitive_inst_conns(Instance *inst) {
  TNode *seq_src, *seq_sink;
  tie(seq_src, seq_sink) = inst->property_value(SEQ_TNODES);
  TData delay;
  if (seq_src && seq_sink) {
    filtered_foreach2(Pin *, ipin, inst->pins(), is_sink(), is_connected()) {
      delay = find_primitive_inst_delay(ipin, &TimingInfo::timing_type,
                                        SETUP_RISING);
      ipin->property_value(TNODE)->create_out_edge(seq_sink, delay._rising,
                                                   delay._falling);
    }
  }

  filtered_foreach2(Pin *, opin, inst->pins(), is_source(), is_connected()) {
    TNode *opin_node = opin->property_value(TNODE);
    if (seq_src && seq_sink) {
      delay = find_primitive_inst_delay(opin, &TimingInfo::timing_type,
                                        RISING_EDGE);
      seq_src->create_out_edge(opin->property_value(TNODE), delay._rising,
                               delay._falling);
    } else {
      filtered_foreach2(Pin *, ipin, inst->pins(), is_sink(), is_connected()) {
        delay = find_primitive_inst_delay(opin, &TimingInfo::related_pin,
                                          ipin->name());
        ipin->property_value(TNODE)->create_out_edge(opin_node, delay._rising,
                                                     delay._falling);
      }
    }
    for (Pin *sink : opin->net()->sink_pins())
      create_conn(opin, sink);
  }
}

void TEngine::create_inst_conns(Instance *inst) {
  if (inst->is_composite())
    create_composite_inst_conns(inst);
  else
    create_primitive_inst_conns(inst);
}

void TEngine::create_all_conns() {
  for (Instance *inst : _target->top_module()->instances())
    create_inst_conns(inst);
}

TData TEngine::find_primitive_inst_delay(Pin *rel_pin,
                                         string TimingInfo::*const tp,
                                         const string &t_value) {
  double r_delay = TPara::ZERO, f_delay = TPara::ZERO;
  auto timings = static_cast<TPort *>(rel_pin->port())->timings();
  auto it = find_if(timings, [tp, &t_value](const TimingInfo &tinfo) {
    return tinfo.*tp == t_value;
  });
  if (it != timings.end()) {
    r_delay = it->intrinsic_rise;
    f_delay = it->intrinsic_fall;
  }
  return TData(r_delay, f_delay);
}

void TEngine::topo_visit(TNode *node) {
  if (node->is_visited())
    return;

  node->set_visited();
  for (const TEdge *oedge : node->out_tedges())
    topo_visit(oedge->to_node());
  _sorted_tnodes.add(node);
}

void TEngine::topo_sort() {
  _sorted_tnodes.clear();

  for (TNode *node : tnodes())
    node->clr_visited();
  for (TNode *node : tnodes())
    topo_visit(node);
}

void TEngine::load_rt_delay() {
  filtered_foreach(Net *, net, _target->top_module()->nets(), num_pins()) {
    TNode *src_tnode = net->source_pins().begin()->property_value(TNODE);
    for (TEdge *o_edge : src_tnode->out_tedges()) {
      TData delay = o_edge->to_node()->owner()->property_value(DELAY);
      o_edge->set_delay(delay);
    }
  }
}

void TEngine::initial_tdata() {
  _max_delay_vec.resize(_t_domains);
  for (TNode *node : tnodes())
    node->resize_tdata(_t_domains);
}

void TEngine::compute_tarr() {
  reversed_foreach(TNode * tnode, sorted_tnodes()) {
    // primary input
    if (!tnode->num_in_tedges())
      tnode->update_pinput_tarrs(t_domain_index(tnode));
    update_max_delays(tnode);

    for (const TEdge *tedge : tnode->out_tedges())
      tedge->to_node()->update_normal_tarrs(tedge);
  }
}

void TEngine::compute_treq() {
  for (TNode *tnode : sorted_tnodes()) {
    // primary output
    if (!tnode->num_out_tedges())
      tnode->update_poutput_treqs(t_domain_index_map(tnode));
    else
      for (const TEdge *oedge : tnode->out_tedges())
        tnode->update_normal_treqs(oedge);
  }
}

void TEngine::compute_slack() {
  for (Net *net : _target->top_module()->nets()) {
    for (Pin *src_pin : net->source_pins()) {
      TNode *src_tnode = src_pin->property_value(TNODE);

      for (TEdge *oedge : src_tnode->out_tedges())
        update_slack_to_pin(oedge);
    }
  }
}

int TEngine::t_domain_index(const TNode *node) const { return _t_domains - 1; }

TEngine::IntTDataMap TEngine::t_domain_index_map(const TNode *node) const {
  IntTDataMap m;
  m.insert(std::make_pair(_t_domains - 1, chief_max_delay()));
  return m;
}

void TEngine::update_max_delays(TNode *cur_tnode) {
  tdata_iter d_it = max_delays().begin();
  for (const TData &t_arr : cur_tnode->t_arrs()) {
    d_it->_rising = max(d_it->_rising, t_arr._rising);
    d_it->_falling = max(d_it->_falling, t_arr._falling);
    ++d_it;
  }
}

void TEngine::update_slack_to_pin(TEdge *tedge) const {
  TDataVec slack_vec(_t_domains);
  tdata_iter slack_it = slack_vec.begin();

  Pin *sink_pin = tedge->to_node()->owner();
  const_tdata_iter sink_treq_it = tedge->to_node()->t_reqs().begin();

  for (const TData &src_tarr : tedge->from_node()->t_arrs()) {
    slack_it->_rising =
        sink_treq_it->_rising - src_tarr._rising - tedge->r_delay();
    slack_it->_falling =
        sink_treq_it->_falling - src_tarr._falling - tedge->f_delay();
    ++slack_it;
    ++sink_treq_it;
  }

  if (_t_domains == TPara::DEFAULT_TDOMAINS)
    sink_pin->set_property(SLACK, slack_vec.begin()->average());
  else { /* add if needed */
  }
}

// create_xml
//	DOM::Element* TNode::create_xml(DOM::Element* root) const { return
// nullptr; } 	DOM::Element* TEdge::create_xml(DOM::Element* root) const {
// return nullptr; }

// echo_dotfile
//	void TNode::echo_dotfile(const TNode& from_node, std::ofstream& ofs)
// const {} 	void TEdge::echo_dotfile(const TNode& from_node, std::ofstream&
// ofs) const {}

} // namespace COS