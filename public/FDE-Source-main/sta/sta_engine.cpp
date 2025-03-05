#include "sta_engine.hpp"
#include "netlist.hpp"

#include <boost/phoenix/core.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/regex.hpp>

// unoptimized, copy from old version

namespace FDU {
namespace STA {

using namespace std;
using namespace boost::adaptors;
using boost::regex;

//////////////////////////////////////////////////////////////////////////
// STAEngine

void STAEngine::timing_analysis() {
  //! <1> get route delay
  calc_routing_delay();

  FDU_LOG(INFO) << (progress_fmt % "60" % "building timing graph").str();
  //! <2> build timing graph
  TFactory::set_factory(new STAEngineFactory());
  create_tgraph();

  //! <3> load routing delay, set primary inputs & outputs
  load_rt_delay();
  store_primary_inputs_and_outputs();

  FDU_LOG(INFO) << (progress_fmt % "70" % "computing arrival time").str();

  //! <4> compute arrive delay
  mark_sequentials(); // mark sequencial cell
  initial_tdata();
  compute_tarr();

  for (const STANode *node : sorted_tnodes() | reversed)
    // set arrive time to pin, arrive time info originally stored in node
    update_tarrs_to_pin(node);
}

void STAEngine::store_primary_inputs_and_outputs() {
  _primary_inputs.clear();
  _primary_outputs.clear();
  for (STANode *node : sorted_tnodes()) {
    if (node->num_in_tedges() == 0)
      _primary_inputs.insert(node);
    else if (node->num_out_tedges() == 0)
      _primary_outputs.insert(node);
  }
}

int STAEngine::find_clock_nets(const Instance &inst) {
  typedef vector<Net *> ClkNets;
  typedef ClkNets::const_iterator const_clknet_iter;
  typedef boost::iterator_range<ClkNets::const_iterator> const_clknets_type;

  int t_idx = 0;
  for (const Pin *pin : inst.pins()) {
    if (!pin->is_connected())
      continue;
    if (pin->name() == "CLK" || pin->name() == "CLKA" ||
        pin->name() == "CLKB" || pin->name() == "CLKIN") // hard code
    {
      const_clknet_iter n_it = boost::find(
          const_clknets_type(_clk_nets.begin(), _clk_nets.end()), pin->net());
      if (n_it == _clk_nets.end()) {
        _clk_nets.push_back(pin->net());
        t_idx = _clk_nets.size() - 1;
      } else {
        t_idx = n_it - _clk_nets.begin();
      }
    }
  }

  return t_idx;
}

void STAEngine::find_seq_primitives(Instance &inst, int t_idx) {
  // inst: slice level
  for (Instance *prim : inst.down_module()->instances()) {
    Instance::pin_iter p_it = find_if(
        prim->pins(), [](const Pin *pin) { return pin->type() == CLOCK; });
    if (p_it != prim->pins().end()) {
      // prim->set_seq(true);
      _dffs.insert(make_pair(prim, t_idx));
    } else {
      ;
      // prim->set_seq(false);
    }
  }
}

void STAEngine::mark_sequentials() {
  _clk_nets.push_back((Net *)nullptr); // for assistance

  for (Instance *inst : target_design()->top_module()->instances())
    find_seq_primitives(*inst, find_clock_nets(*inst));

  set_t_domains(_clk_nets.size());
}

void STAEngine::update_tarrs_to_pin(const STANode *sta_node) {
  Property<vector<TimingPoint>> &TP =
      create_temp_property<vector<TimingPoint>>(PIN, "timingpoint");
  vector<TimingPoint> tarrs;
  for (const TData tarr : sta_node->t_arrs()) {
    tarrs.push_back(TimingPoint(tarr._rising, tarr._falling));
  }
  sta_node->owner()->set_property(TP, tarrs);
}

int STAEngine::t_domain_index(const TNode *node) const {
  int t_idx = 0;
  if (node->type() == TNode::SEQ_SRC) {
    Instance *inst = node->owner()->instance();
    ASSERTD(_dffs.count(inst), "can NOT find corresponding dff.");
    t_idx = _dffs.find(inst)->second;
  } else { // for CONST_GEN_SRC or IN_PORT(cell_pin)
    t_idx = 0;
  }
  return t_idx;
}

// find longest path which ends with po
// when meet a pi, the function will return
void STAEngine::trace_from_primary_output(STANode *po, list<STANode *> &path,
                                          bool rise, int i) {
  path.clear();
  path.push_back(po);

  STANode *cur_node = po;
  while (!_primary_inputs.count(cur_node)) {
    for (const TEdge *iedge : cur_node->in_tedges()) {
      TData from_tdata = static_cast<STANode *>(iedge->from_node())->t_arr(i);
      double from_tarr = rise ? from_tdata._rising : from_tdata._falling;
      double from_delay = rise ? iedge->r_delay() : iedge->f_delay();
      double cur_tarr =
          rise ? cur_node->t_arr(i)._rising : cur_node->t_arr(i)._falling;
      if ((from_tarr + from_delay) - cur_tarr > -1e-8) {
        cur_node = static_cast<STANode *>(iedge->from_node());
        path.push_back(cur_node);
        break;
      }
    }
  }
}

} // namespace STA
} // namespace FDU
