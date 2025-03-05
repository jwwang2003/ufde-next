#include "Match.h"
#include "PKFactory.h"

namespace PACK {

using namespace std;

//////////////////////////////////////////////////////////////////////////
//  constructors
PKNet::PKNet(const string &name, NetType type, Module *owner, Bus *bus)
    : Net(name, type, owner, bus), num_pure_sinks_(-1), touch_flag_(false),
      is_used_(false) {}
PKCell::PKCell(const string &name, const string &type, Library *owner)
    : Module(name, type, owner), is_graph_update_(false) {}
PKInstance::PKInstance(const string &name, Module *down_module, Module *owner)
    : Instance(name, down_module, owner), gobj_(nullptr), vcell_(nullptr),
      is_used_(false) {}
PKPin::PKPin(const string &name, Port *port, Module *owner, int index,
             int mindex)
    : Pin(name, port, owner, index, mindex), is_used_(false) {}
PKPin::PKPin(Pin *mpin, Instance *owner) : Pin(mpin, owner), is_used_(false) {}
PKPort::PKPort(const string &name, int msb, int lsb, DirType dir, PortType type,
               bool is_vec, Module *owner, int pin_from)
    : Port(name, msb, lsb, dir, type, is_vec, owner, pin_from),
      port_class_(INVALID) {}

RulePort::RulePort(const string &name, int msb, int lsb, DirType dir,
                   PortType type, bool is_vec, Module *owner, int pin_from)
    : Port(name, msb, lsb, dir, type, is_vec, owner, pin_from),
      is_connect_(false), is_bus_ignore_(false) {}
RuleInstance::RuleInstance(const string &name, Module *down_module,
                           Module *owner)
    : PKInstance(name, down_module, owner), rule_lock_(true),
      image_inst_(nullptr) {}
RuleCell::RuleCell(const string &name, const string &type, Library *owner)
    : PKCell(name, type, owner), num_lut_(0), num_ff_(0) {}

//////////////////////////////////////////////////////////////////////////
//  factory
Net *PKFactory::make_net(const string &name, NetType type, Module *owner,
                         Bus *bus) {
  return new PKNet(name, type, owner, bus);
}
Module *PKFactory::make_module(const string &name, const string &type,
                               Library *owner) {
  return new PKCell(name, type, owner);
}
Instance *PKFactory::make_instance(const string &name, Module *down_module,
                                   Module *owner) {
  return new PKInstance(name, down_module, owner);
}
Pin *PKFactory::make_pin(const string &name, Port *port, Module *owner,
                         int index, int mindex) {
  return new PKPin(name, port, owner, index, mindex);
}
Pin *PKFactory::make_pin(Pin *mpin, Instance *owner) {
  return new PKPin(mpin, owner);
}
Port *PKFactory::make_port(const string &name, int msb, int lsb, DirType dir,
                           PortType type, bool is_vec, Module *owner,
                           int pin_from) {
  return new PKPort(name, msb, lsb, dir, type, is_vec, owner, pin_from);
}

Module *RuleFactory::make_module(const string &name, const string &type,
                                 Library *owner) {
  return new RuleCell(name, type, owner);
}
Instance *RuleFactory::make_instance(const string &name, Module *down_module,
                                     Module *owner) {
  return new RuleInstance(name, down_module, owner);
}
Port *RuleFactory::make_port(const string &name, int msb, int lsb, DirType dir,
                             PortType type, bool is_vec, Module *owner,
                             int pin_from) {
  return new RulePort(name, msb, lsb, dir, type, is_vec, owner, pin_from);
}

//////////////////////////////////////////////////////////////////////////
// PKNet

SimpleNet &PKNet::get_simple_net(PKPin *p, PKInstance *i) {
  SimpleNet sn(p, i);
  SimpleNets::iterator it = boost::find(simple_nets(), sn);
  if (it != simple_nets().end())
    return *it;
  smpl_nets_.push_back(sn);
  return smpl_nets_.back();
}

void PKNet::parse() {
  num_pure_sinks_ = 0;
  for (PKPin *pin : sink_pins())
    if (!pin->is_mpin())
      ++num_pure_sinks_;
    else
      sink_cell_pins_.push_back(pin);

  smpl_nets_.clear();
  for (PKPin *src : source_pins())
    for (PKPin *sink : sink_pins())
      if (!sink->is_mpin())
        get_simple_net(src, sink->instance()).add_sink(sink);
}

void PKNet::add_vpin(VPin *vp) {
  vpins_.push_back(vp);
  unique_vcell_.insert(vp->vport()->owner());
}

void PKNet::remove_vpin(VPin *vp) {
  vpins_.remove(vp);
  for (VPin *vpin : vpins_)
    if (vpin->vport()->owner() == vp->vport()->owner())
      return;
  unique_vcell_.erase(vp->vport()->owner());
}

//////////////////////////////////////////////////////////////////////////
//  PKCell
CellGraph *PKCell::build_graph() {
  CellGraph *g = new CellGraph(this);
  g->build_graph();
  rela_graph_.reset(g);
  return g;
}

void PKCell::parse_nets() {
  for (PKNet *net : nets()) {
    /*if (net.name()=="IO")
    {
            cout<<"Net name: "<<net.name()<<"\nSink pins:
    "<<net.sink_pins()<<"\nSrc pins: "<<net.source_pins()<<endl;
    }*/

    net->parse();
  }
}

void PKCell::after_inst_created(PKInstance *inst) {
  if (is_graph_update_ && rela_graph_.get() != nullptr)
    rela_graph_->after_inst_created(inst);
}

void PKCell::before_pin_unhook(PKNet *net, PKPin *pin) {
  if (is_graph_update_ && rela_graph_.get() != nullptr)
    rela_graph_->before_pin_unhooked(net, pin);
}

void PKCell::before_pin_hookup(PKNet *net, PKPin *pin) {
  if (is_graph_update_ && rela_graph_.get() != nullptr)
    rela_graph_->before_pin_hookuped(net, pin);
}

void PKCell::before_pin_rehook(PKNet *old_net, PKNet *new_net, PKPin *pin) {
  if (is_graph_update_ && rela_graph_.get() != nullptr)
    rela_graph_->before_pin_rehook(old_net, new_net, pin);
}

void PKCell::before_inst_remove(PKInstance *inst) {
  if (is_graph_update_ && rela_graph_.get() != nullptr)
    rela_graph_->before_inst_remove(inst);
}

void PKCell::before_net_remove(PKNet *net) {
  if (is_graph_update_ && rela_graph_.get() != nullptr)
    rela_graph_->before_net_remove(net);
}

//////////////////////////////////////////////////////////////////////////
// RuleCell
void RuleCell::cnt_resource() {
  for (Instance *inst : instances()) {
    if (inst->down_module()->name() == CELL_NAME::LUT)
      ++(num_lut_);
    else if (inst->down_module()->name() == CELL_NAME::FF)
      ++(num_ff_);
  }
}

} // namespace PACK