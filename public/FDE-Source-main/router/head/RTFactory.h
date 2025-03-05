#ifndef _RTFACTORY_H
#define _RTFACTORY_H

#include "RTCstLoadHandler.h"
#include "rtnetlist.hpp"
#include <list>

namespace FDU {
namespace RT {

using namespace COS;

class RTNode;
class RTGraph;

class RTNet : public COSRTNet {
public:
  struct BBFac {
    Point bblt;
    Point bbrb;
  };
  typedef std::list<RTNode *>::iterator path_node_iter;
  typedef std::list<RTNode *>::const_iterator const_path_node_iter;

public:
  RTNet(const string &name, NetType type, Module *owner, Bus *bus)
      : COSRTNet(name, type, owner, bus), is_ignore_(false),
        is_partial_routed_(false), src_node_(nullptr), seg_begin_(path_.end()),
        seg_end_(path_.end()) {}

  void init_rt_info(RTGraph *, int, const CstNets *);
  void path_update_one_cost(path_node_iter, int, double);
  bool is_feasible();
  void save_path();

  path_node_iter update_trace_back(RTNode *);
  void free_path_back() {
    path_.clear();
    seg_begin_ = seg_end_ = path_.end();
  }
  const BBFac &bb_fac() const { return bb_; }

  RTNode *src_node() const { return src_node_; }
  std::vector<RTNode *> &sink_nodes() { return sink_nodes_; }
  void mark_sinks();

  path_node_iter path_begin() { return path_.begin(); }
  path_node_iter path_end() { return path_.end(); }

  bool is_ignore() const { return is_ignore_; }
  /*		void process_vdd_gnd();*/
  void load_info_for_routed_net(RTGraph *);
  bool is_completely_routed(RTGraph *);
  void load_info_from_fixed_path();
  bool is_partial_routed() { return is_partial_routed_; }

private:
  void load_bb_fac(int);
  void load_src_sink_nodes(RTGraph *);

private:
  bool is_ignore_;
  bool is_partial_routed_;
  BBFac bb_;
  RTNode *src_node_;
  std::vector<RTNode *> sink_nodes_;

  std::list<RTNode *> path_;
  std::list<RTNode *> fixed_path_;
  path_node_iter seg_end_;
  path_node_iter seg_begin_;
};

class RTCell : public Module {
public:
  RTCell(const string &name, const string &type, Library *owner)
      : Module(name, type, owner) {}

  typedef PtrVector<Net>::typed<RTNet>::range nets_type;
  typedef PtrVector<Net>::typed<RTNet>::const_range const_nets_type;
  typedef PtrVector<Net>::typed<RTNet>::iterator net_iter;
  typedef PtrVector<Net>::typed<RTNet>::const_iterator const_net_iter;

  nets_type nets() { return static_cast<nets_type>(Module::nets()); }
  const_nets_type nets() const {
    return static_cast<const_nets_type>(Module::nets());
  }
};

class RTFactory : public COSRTFactory {
public:
  Module *make_module(const string &name, const string &type, Library *owner) {
    return new RTCell(name, type, owner);
  }
  Net *make_net(const string &name, NetType type, Module *owner, Bus *bus) {
    return new RTNet(name, type, owner, bus);
  }
};

} // namespace RT
} // namespace FDU

#endif