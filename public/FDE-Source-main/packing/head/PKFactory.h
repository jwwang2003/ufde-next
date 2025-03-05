/*! \file PKFactory.h
 *  \brief  Header of packing's netlist data structure
 *  \author S.C. Leung
 *  \date   2009.05 - 2009.06
 *
 */

#ifndef _PKFACTORY
#define _PKFACTORY

#include "netlist.hpp"

namespace PACK {

using std::string;

using namespace COS;

class PKInstance;
class PKNet;
class PKPin;
class GNode;
class CellGraph;
class Rule;
class VCell;
class VPort;
class VPin;

class GNode;
class GArc;

class PKPin : public Pin {
public:
  PKPin(const string &name, Port *port, Module *owner, int index, int mindex);
  PKPin(Pin *mpin, Instance *owner);

  PKInstance *instance() const;
  PKNet *net() const;
  bool is_used() const { return is_used_; }

  void set_used() { is_used_ = true; }
  void clear_used() { is_used_ = false; }

private:
  bool is_used_;
};

/*sophie begin:JULY 28TH*/
class PKPort : public Port {
public:
  PKPort(const string &name, int msb, int lsb, DirType dir, PortType type,
         bool is_vec, Module *owner, int pin_from);

  int port_class() const { return port_class_; }
  void set_port_class(int _port_class) { port_class_ = _port_class; }

private:
  int port_class_;
}; /*sophie end:for the purpose of adding port class info*/

struct SimpleNet {
  PKPin *src;
  PKInstance *inst;
  std::vector<PKPin *> sinks;
  SimpleNet(PKPin *p, PKInstance *i) : src(p), inst(i) {}
  void add_sink(PKPin *pin) {
    ASSERTSD(inst == pin->instance());
    sinks.push_back(pin);
  }
  bool operator==(const SimpleNet &rhs) const {
    return src == rhs.src && inst == rhs.inst;
  }
};

class PKNet : public Net {
  friend class PKCell;
  friend class VPin;

public:
  PKNet(const string &name, NetType type, Module *owner, Bus *bus);

  // GObjects
  typedef boost::iterator_range<std::list<GArc *>::const_iterator> gobjs_type;

  void remove_gobj(GArc *o) { gobjs_.remove(o); }
  void add_gobj(GArc *o) { gobjs_.push_back(o); }
  gobjs_type gobjects() { return gobjs_type(gobjs_.begin(), gobjs_.end()); }

  // Pin
  typedef PtrVector<Pin, false>::typed<PKPin>::range pins_type;
  typedef PtrVector<Pin, false>::typed<PKPin>::const_range const_pins_type;
  typedef PtrVector<Pin, false>::typed<PKPin>::iterator pin_iter;
  typedef PtrVector<Pin, false>::typed<PKPin>::const_iterator const_pin_iter;

  pins_type pins() { return static_cast<pins_type>(Net::pins()); }
  const_pins_type pins() const {
    return static_cast<const_pins_type>(Net::pins());
  }
  pins_type source_pins() { return static_cast<pins_type>(Net::source_pins()); }
  const_pins_type source_pins() const {
    return static_cast<const_pins_type>(Net::source_pins());
  }
  pins_type sink_pins() { return static_cast<pins_type>(Net::sink_pins()); }
  const_pins_type sink_pins() const {
    return static_cast<const_pins_type>(Net::sink_pins());
  }
  PKPin *find_pin(const string &name) {
    return static_cast<PKPin *>(Net::find_pin(name));
  }

  // SimpleNet & SinkCellPin
  typedef std::vector<PKPin *> PinVec;
  typedef std::vector<SimpleNet> SimpleNets;
  typedef boost::iterator_range<PinVec::const_iterator> const_ppins_type;
  typedef boost::iterator_range<SimpleNets::iterator> smplnets_type;
  typedef boost::iterator_range<SimpleNets::const_iterator> const_smplnets_type;

  int inc_pure_sinks() { return ++num_pure_sinks_; }
  int dec_pure_sinks() { return --num_pure_sinks_; }
  int num_pure_sinks() const { return num_pure_sinks_; }
  const_ppins_type sink_cell_pins() const {
    return const_ppins_type(sink_cell_pins_.begin(), sink_cell_pins_.end());
  }
  smplnets_type simple_nets() {
    return smplnets_type(smpl_nets_.begin(), smpl_nets_.end());
  }
  const_smplnets_type simple_nets() const {
    return const_smplnets_type(smpl_nets_.begin(), smpl_nets_.end());
  }
  SimpleNet &get_simple_net(PKPin *p, PKInstance *i);

  // VPin
  typedef boost::iterator_range<std::list<VPin *>::const_iterator>
      const_vpins_type;
  typedef boost::iterator_range<std::list<VPin *>::iterator> vpins_type;

  size_t num_vpins() const { return vpins_.size(); }
  const_vpins_type vpins() const {
    return const_vpins_type(vpins_.begin(), vpins_.end());
  }
  vpins_type vpins() { return vpins_type(vpins_.begin(), vpins_.end()); }

  // Unique VCell
  typedef boost::iterator_range<unique_list<VCell *>::const_iterator>
      const_vcells_type;
  const_vcells_type unique_vcells() const {
    return const_vcells_type(unique_vcell_.begin(), unique_vcell_.end());
  }

  // Clustering Interface
  bool is_touch() const { return touch_flag_; }
  void set_touch_flag() { touch_flag_ = true; }
  void clear_touch_flag() { touch_flag_ = false; }
  bool is_used() const { return is_used_; } // sophie
  void set_used() { is_used_ = true; }      // sophie

  // void inc_num_vpin_in_cluster(int i) { num_vpin_in_cluster_ += i; }
  // void reset_num_vpin_in_cluster()    { num_vpin_in_cluster_  = 0; }

private:
  void parse();
  void add_vpin(VPin *vp);
  void remove_vpin(VPin *vp);

  std::list<GArc *> gobjs_;
  PinVec sink_cell_pins_;
  SimpleNets smpl_nets_;
  int num_pure_sinks_;

  bool touch_flag_;
  bool is_used_; // sophie
  // int              num_vpin_in_cluster_;
  std::list<VPin *> vpins_;
  unique_list<VCell *> unique_vcell_;
};

class PKInstance : public Instance {
public:
  PKInstance(const string &name, Module *down_module, Module *owner);

  // CellGraph relative interface
  GNode *gobject() const { return gobj_; }
  void set_gobj(GNode *o) { gobj_ = o; }

  // Virtual Module relative interface
  VCell *vcell() const { return vcell_; }
  void set_vcell(VCell *c) { vcell_ = c; }
  bool is_used() const { return is_used_; }
  void set_used() { is_used_ = true; }
  void clear_used() { is_used_ = false; }

  //  Pin
  typedef PtrVector<Pin>::typed<PKPin>::range pins_type;
  typedef PtrVector<Pin>::typed<PKPin>::const_range const_pins_type;
  typedef PtrVector<Pin>::typed<PKPin>::iterator pin_iter;
  typedef PtrVector<Pin>::typed<PKPin>::const_iterator const_pin_iter;

  const_pins_type pins() const {
    return static_cast<const_pins_type>(Instance::pins());
  }
  pins_type pins() { return static_cast<pins_type>(Instance::pins()); }
  PKPin *find_pin(const string &name) {
    return static_cast<PKPin *>(Instance::find_pin(name));
  }

private:
  GNode *gobj_;
  VCell *vcell_;
  bool is_used_;
};

class PKCell : public Module {
public:
  PKCell(const string &name, const string &type, Library *owner);

  // PKInstance
  typedef PtrList<Instance>::typed<PKInstance>::range instances_type;
  typedef PtrList<Instance>::typed<PKInstance>::const_range
      const_instances_type;
  typedef PtrList<Instance>::typed<PKInstance>::iterator inst_iter;
  typedef PtrList<Instance>::typed<PKInstance>::const_iterator const_inst_iter;

  instances_type instances() {
    return static_cast<instances_type>(Module::instances());
  }
  const_instances_type instances() const {
    return static_cast<const_instances_type>(Module::instances());
  }
  PKInstance *find_instance(const string &name) {
    return static_cast<PKInstance *>(Module::find_instance(name));
  }

  // PKNet
  typedef PtrVector<Net>::typed<PKNet>::range nets_type;
  typedef PtrVector<Net>::typed<PKNet>::const_range const_nets_type;
  typedef PtrVector<Net>::typed<PKNet>::iterator net_iter;
  typedef PtrVector<Net>::typed<PKNet>::const_iterator const_net_iter;

  const_nets_type nets() const {
    return static_cast<const_nets_type>(Module::nets());
  }
  nets_type nets() { return static_cast<nets_type>(Module::nets()); }
  PKNet *find_net(const string &name) {
    return static_cast<PKNet *>(Module::find_net(name));
  }
  void parse_nets();

  // CellGraph
  CellGraph *relative_graph() const { return rela_graph_.get(); }
  void enable_update_graph() { is_graph_update_ = true; }
  void stop_update_graph() { is_graph_update_ = false; }
  CellGraph *build_graph();

  // signal about cell change
  void after_inst_created(PKInstance *inst);
  void before_pin_unhook(PKNet *net, PKPin *pin);
  void before_pin_hookup(PKNet *net, PKPin *pin);
  void before_pin_rehook(PKNet *old_net, PKNet *new_net, PKPin *pin);
  void before_inst_remove(PKInstance *inst);
  void before_net_remove(PKNet *net);

private:
  boost::scoped_ptr<CellGraph> rela_graph_;
  bool is_graph_update_;
};

inline PKInstance *PKPin::instance() const {
  return static_cast<PKInstance *>(Pin::instance());
}
inline PKNet *PKPin::net() const { return static_cast<PKNet *>(Pin::net()); }

class PKFactory : public CosFactory {
public:
  Net *make_net(const string &name, NetType type, Module *owner, Bus *bus);
  Module *make_module(const string &name, const string &type, Library *owner);
  Instance *make_instance(const string &name, Module *down_module,
                          Module *owner);
  Pin *make_pin(const string &name, Port *port, Module *owner, int index,
                int mindex);
  Pin *make_pin(Pin *mpin, Instance *owner);
  Port *make_port(const string &name, int msb, int lsb, DirType dir,
                  PortType type, bool is_vec, Module *owner, int pin_from);
};

class RulePort : public Port {
public:
  RulePort(const string &name, int msb, int lsb, DirType dir, PortType type,
           bool is_vec, Module *owner, int pin_from);

  bool is_connect() const { return is_connect_; }
  void set_connect() { is_connect_ = true; }
  bool is_bus_ignore() const { return is_bus_ignore_; }
  void set_bus_ignore() { is_bus_ignore_ = true; }

private:
  bool is_connect_;
  bool is_bus_ignore_;
};

class RuleInstance : public PKInstance {
public:
  RuleInstance(const string &name, Module *down_module, Module *owner);

  bool rule_lock() const { return rule_lock_; }
  void clear_rule_lock() { rule_lock_ = false; }

  PKInstance *image() const { return image_inst_; }
  void set_image(PKInstance *i) { image_inst_ = i; }

private:
  bool rule_lock_;
  PKInstance *image_inst_;
};

class RuleCell : public PKCell {
public:
  RuleCell(const string &name, const string &type, Library *owner);

  // Interface for RuleCell
  Rule *rule() const { return co_rule_; }
  int num_lut() const { return num_lut_; }
  int num_ff() const { return num_ff_; }
  void cnt_resource();

  //  RuleInstances
  typedef PtrList<Instance>::typed<RuleInstance>::range instances_type;
  typedef PtrList<Instance>::typed<RuleInstance>::const_range
      const_instances_type;
  typedef PtrList<Instance>::typed<RuleInstance>::iterator inst_iter;
  typedef PtrList<Instance>::typed<RuleInstance>::const_iterator
      const_inst_iter;

  instances_type instances() {
    return static_cast<instances_type>(Module::instances());
  }
  const_instances_type instances() const {
    return static_cast<const_instances_type>(Module::instances());
  }
  RuleInstance *find_instance(const string &name) {
    return static_cast<RuleInstance *>(Module::find_instance(name));
  }

  // Port
  typedef PtrVector<Port>::typed<RulePort>::range ports_type;
  typedef PtrVector<Port>::typed<RulePort>::const_range const_ports_type;
  typedef PtrVector<Port>::typed<RulePort>::iterator port_iter;
  typedef PtrVector<Port>::typed<RulePort>::const_iterator const_port_iter;

  ports_type ports() { return static_cast<ports_type>(Module::ports()); }
  const_ports_type ports() const {
    return static_cast<const_ports_type>(Module::ports());
  }
  RulePort *find_port(const string &name) {
    return static_cast<RulePort *>(Module::find_port(name));
  }

private:
  friend class Rule;
  Rule *co_rule_;
  int num_lut_;
  int num_ff_;
};

class RuleFactory : public PKFactory {
public:
  Module *make_module(const string &name, const string &type, Library *owner);
  Instance *make_instance(const string &name, Module *down_module,
                          Module *owner);
  Port *make_port(const string &name, int msb, int lsb, DirType dir,
                  PortType type, bool is_vec, Module *owner, int pin_from);
};

} // namespace PACK

#endif
