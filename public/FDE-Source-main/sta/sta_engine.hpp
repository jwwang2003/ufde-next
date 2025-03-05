#ifndef STA_ENGINE_HPP
#define STA_ENGINE_HPP

#include "rd_engine.hpp"
#include "sta_arg.hpp"
#include "sta_utils.hpp"
#include "tengine/tengine.hpp"

#include <list>

// unoptimized, copy from old version

using namespace ::COS;

namespace FDU {
namespace STA {

struct TimingPoint {
  double _arrival_rise_late;
  double _arrival_fall_late;

  TimingPoint(double r_arr = 0., double f_arr = 0.)
      : _arrival_rise_late(r_arr), _arrival_fall_late(f_arr) {}
};

class STANode : public TNode {
public:
  STANode(TNodeType type, Pin *owner, int index) : TNode(type, owner, index) {}

  virtual void update_pinput_tarrs(int t_index) {
    TNode::update_pinput_tarrs(t_index);
  }
  virtual void update_normal_tarrs(const TEdge *f_tedge) {
    TNode::update_normal_tarrs(f_tedge);
  }
};

class STAEngineFactory : public TFactory {
  virtual TNode *make_tnode(TNode::TNodeType type, Pin &owner, int index) {
    return new STANode(type, &owner, index);
  }
};

class STAEngine : public TEngine {
  // convenience for export report
  friend class STAReport;

public:
  STAEngine(TDesign *target, STAArg *arg) : TEngine(target), arg_(arg) {}

  void timing_analysis();

  void add_test_inst();

protected:
  typedef TNodeVec::typed<STANode>::iterator sorted_tnode_iter;
  typedef TNodeVec::typed<STANode>::const_iterator const_sorted_tnode_iter;
  typedef TNodeVec::typed<STANode>::range sorted_tnodes_type;
  typedef TNodeVec::typed<STANode>::const_range const_sorted_tnodes_type;

  sorted_tnodes_type sorted_tnodes() {
    return static_cast<sorted_tnodes_type>(TEngine::sorted_tnodes());
  }
  const_sorted_tnodes_type sorted_tnodes() const {
    return static_cast<const_sorted_tnodes_type>(TEngine::sorted_tnodes());
  }

  void store_primary_inputs_and_outputs();
  int find_clock_nets(const Instance &inst);
  void find_seq_primitives(Instance &inst, int t_idx);
  void mark_sequentials();
  void update_tarrs_to_pin(const STANode *sta_node);

  void calc_routing_delay() {
    FDU_LOG(INFO) << (progress_fmt % 35 % "begin analysis routing delay").str();
    RDEngine rd_engine(target_design(), arg_); // local variable to save memory
    rd_engine.compute_route_delay();
  }

  // for report
  void trace_from_primary_output(STANode *po, std::list<STANode *> &path,
                                 bool rise, int i);

  virtual int t_domain_index(const TNode *node) const; // for primary input

private:
  std::set<STANode *> _primary_inputs;
  std::set<STANode *> _primary_outputs;

  std::vector<Net *> _clk_nets;
  std::map<Instance *, int> _dffs;
  STAArg *arg_;
};

} // namespace STA
} // namespace FDU

#endif
