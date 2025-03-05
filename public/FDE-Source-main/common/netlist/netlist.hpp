#ifndef NETLIST_HPP
#define NETLIST_HPP

#include "object.hpp"
#include "ptrcontainer.hpp"
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace COS {
using std::vector;

enum DirType { DIR_IGNORE = 0, INPUT, OUTPUT, INOUT };
enum PortType { NORMAL = 0, CLOCK, RESET, ENABLE, CARRY, VCC, TBUF };
using NetType = PortType;
enum ConnectDir { CONN_LEFT, CONN_RIGHT };

class Design;
class Library;
class Module;
class Instance;
class Port;
class Net;
class Bus;
class Pin;
namespace XML {
class NetlistHandler;
class NetlistWriter;
} // namespace XML

///////////////////////////////////////////////////////////////////////////////////////
//  CosFactory
class CosFactory {
public:
  using pointer = std::unique_ptr<CosFactory>;

  static CosFactory &instance() { return *_instance.get(); }
  static pointer set_factory(pointer &f) {
    pointer p = std::move(_instance);
    _instance = std::move(f);
    return p;
  }
  static pointer set_factory(CosFactory *f) {
    pointer p{f};
    return set_factory(p);
  }

  virtual ~CosFactory() {}
  //		virtual Design*   make_design(const string& name);
  virtual Library *make_library(const string &name, Design *owner, bool extrn);
  virtual Module *make_module(const string &name, const string &type,
                              Library *owner);
  virtual Instance *make_instance(const string &name, Module *down_module,
                                  Module *owner);
  virtual Net *make_net(const string &name, NetType type, Module *owner,
                        Bus *bus);
  virtual Bus *make_bus(const string &name, Module *owner, int msb, int lsb);
  virtual Port *make_port(const string &name, int msb, int lsb, DirType dir,
                          PortType type, bool is_vec, Module *owner,
                          int pin_from);
  virtual Pin *make_pin(const string &name, Port *port, Module *owner,
                        int index, int mindex);
  virtual Pin *make_pin(Pin *mpin, Instance *owner);

  virtual XML::NetlistHandler *make_xml_read_handler();
  virtual XML::NetlistWriter *make_xml_write_handler();

private:
  static pointer _instance;
};

///////////////////////////////////////////////////////////////////////////////////////
//  Design
class Design : public Object {
public:
  explicit Design(const string &name = "") : Object(name, DESIGN, 0), _top(0) {}

  void set_top_module(Module *top) { _top = top; }
  Module *top_module() const { return _top; }
  Library *work_lib() const; // { return _top ? _top->library() : 0; }

  using libs_type = PtrVector<Library>::range_type;
  using const_libs_type = PtrVector<Library>::const_range_type;
  using lib_iter = PtrVector<Library>::iterator;
  using const_lib_iter = PtrVector<Library>::const_iterator;

  Library *create_library(const string &name, bool extrn = false) {
    return _libs.add(CosFactory::instance().make_library(name, this, extrn));
  }
  Library *find_library(const string &name) { return libs().find(name); }
  const Library *find_library(const string &name) const {
    return libs().find(name);
  }
  Library *find_or_create_library(const string &name, bool extrn = false) {
    if (Library *lib = find_library(name))
      return lib;
    else
      return create_library(name, extrn);
  }
  lib_iter remove_library(lib_iter cit) { return _libs.erase(cit); }
  void remove_library(Library *lib) { _libs.erase(lib); }

  std::size_t num_libs() const { return _libs.size(); }
  libs_type libs() { return _libs.range(); }
  const_libs_type libs() const { return _libs.range(); }

  void load(const string &type, std::istream &src);
  void load(const string &type, const std::string &src);
  void save(const string &type, std::ostream &ost);
  void save(const string &type, const string &file, bool encrypt = false);

  string path_name() const { return "/"; }

private:
  string _chip_type;
  Module *_top;
  PtrVector<Library> _libs;
};

///////////////////////////////////////////////////////////////////////////////////////
//  Library
class Library : public Object {
public:
  explicit Library(const string &name, Design *owner = 0, bool extrn = false)
      : Object(name, LIBRARY, owner), _external(extrn) {}
  Design *design() const { return static_cast<Design *>(owner()); }

  using modules_type = PtrVector<Module>::range_type;
  using const_modules_type = PtrVector<Module>::const_range_type;
  using module_iter = PtrVector<Module>::iterator;
  using const_module_iter = PtrVector<Module>::const_iterator;

  Module *create_module(const string &cell_name, const string &cell_type = "") {
    return _modules.add(
        CosFactory::instance().make_module(cell_name, cell_type, this));
  }
  module_iter remove_module(module_iter cit) { return _modules.erase(cit); }
  void remove_module(Module *module) { _modules.erase(module); }
  Module *find_module(const string &name) { return modules().find(name); }
  const Module *find_module(const string &name) const {
    return modules().find(name);
  }

  std::size_t num_modules() const { return _modules.size(); }
  modules_type modules() { return _modules.range(); }
  const_modules_type modules() const { return _modules.range(); }

  string path_name() const { return "/lib:" + name(); }
  bool is_external() const { return _external; }
  void set_external(bool x) { _external = x; }

private:
  bool _external;
  PtrVector<Module> _modules;
};

///////////////////////////////////////////////////////////////////////////////////////
//  Module
class Module : public Object {
public:
  Module(const string &name, const string &type, Library *owner);

  Library *library() const { return static_cast<Library *>(owner()); }
  const string &type() const { return _type; }

  //  Ports
  using ports_type = PtrVector<Port>::range_type;
  using const_ports_type = PtrVector<Port>::const_range_type;
  using port_iter = PtrVector<Port>::iterator;
  using const_port_iter = PtrVector<Port>::const_iterator;

  Port *create_port(const string &name, int msb, int lsb, DirType dir,
                    PortType type = NORMAL);
  Port *create_port(const string &name, DirType dir, PortType type = NORMAL);
  port_iter remove_port(port_iter pit);
  void remove_port(Port *port) {
    remove_port(boost::range::find(ports(), port));
  }
  Port *find_port(const string &name) { return ports().find(name); }
  const Port *find_port(const string &name) const { return ports().find(name); }

  std::size_t num_ports() const { return _ports.size(); }
  ports_type ports() { return _ports.range(); }
  const_ports_type ports() const { return _ports.range(); }

  using pins_type = PtrVector<Pin>::range_type;
  using const_pins_type = PtrVector<Pin>::const_range_type;
  using pin_iter = PtrVector<Pin>::iterator;
  using const_pin_iter = PtrVector<Pin>::const_iterator;

  Pin *find_pin(const string &name) { return pins().find(name); }
  const Pin *find_pin(const string &name) const { return pins().find(name); }

  std::size_t num_pins() const { return _pins.size(); }
  Pin *pin(int i) { return _pins[i]; }
  const Pin *pin(int i) const { return _pins[i]; }
  pins_type pins() { return _pins.range(); }
  const_pins_type pins() const { return _pins.range(); }
  pins_type pins(const Port *port);
  const_pins_type pins(const Port *port) const;

  //  Instances
  using instances_type = PtrList<Instance>::range_type;
  using const_instances_type = PtrList<Instance>::const_range_type;
  using inst_iter = PtrList<Instance>::iterator;
  using const_inst_iter = PtrList<Instance>::const_iterator;

  Instance *create_instance(const string &inst_name, Module *down_module);
  std::pair<Instance *, inst_iter>
  create_instance(const string &inst_name, Module *down_module, inst_iter iit);
  inst_iter remove_instance(inst_iter iit);
  void remove_instance(Instance *inst);
  Instance *find_instance(const string &name) { return instances().find(name); }
  const Instance *find_instance(const string &name) const {
    return instances().find(name);
  }

  std::size_t num_instances() const { return _instances.size(); }
  instances_type instances() { return _instances.range(); }
  const_instances_type instances() const { return _instances.range(); }

  std::size_t num_up_instances() const { return _up_instances.size(); }
  instances_type up_instances() { return _up_instances.range(); }
  const_instances_type up_instances() const { return _up_instances.range(); }

  Instance *find_up_instance(const string &name) {
    return up_instances().find(name);
  }
  const Instance *find_up_instance(const string &name) const {
    return up_instances().find(name);
  }

  //  Nets
  using nets_type = PtrVector<Net>::range_type;
  using const_nets_type = PtrVector<Net>::const_range_type;
  using net_iter = PtrVector<Net>::iterator;
  using const_net_iter = PtrVector<Net>::const_iterator;

  Net *create_net(const string &net_name, NetType type = NORMAL) {
    return _nets.add(CosFactory::instance().make_net(net_name, type, this, 0));
  }
  net_iter remove_net(net_iter nit) { return _nets.erase(nit); }
  void remove_net(Net *net) { _nets.erase(net); }
  Net *find_net(const string &name) { return nets().find(name); }
  const Net *find_net(const string &name) const { return nets().find(name); }

  std::size_t num_nets() const { return _nets.size(); }
  nets_type nets() { return _nets.range(); }
  const_nets_type nets() const { return _nets.range(); }

  //  Buses
  using buses_type = PtrVector<Bus>::range_type;
  using const_buses_type = PtrVector<Bus>::const_range_type;
  using bus_iter = PtrVector<Bus>::iterator;
  using const_bus_iter = PtrVector<Bus>::const_iterator;

  Bus *create_bus(const string &name, int msb, int lsb, NetType type = NORMAL);
  bus_iter remove_bus(bus_iter nit);
  void remove_bus(Bus *bus) { remove_bus(boost::range::find(buses(), bus)); }
  Bus *find_bus(const string &name) { return buses().find(name); }
  const Bus *find_bus(const string &name) const { return buses().find(name); }

  std::size_t num_buses() const { return _buses.size(); }
  buses_type buses() { return _buses.range(); }
  const_buses_type buses() const { return _buses.range(); }

  Module *clone(const string &new_name) { return clone(new_name, library()); }
  Module *clone(const string &new_name, Library *new_owner);
  void flatten();
  bool is_composite() const { return num_nets() + num_instances(); }
  string path_name() const {
    return library()->path_name() + "/module:" + name();
  }

private:
  int pin_from(const Port *port) const; // { return port->_pin_from; }
  void renumber_ports() const;

  string _type;
  PtrVector<Port> _ports;
  PtrVector<Pin> _pins;
  PtrList<Instance> _instances;
  PtrList<Instance, false> _up_instances;
  PtrVector<Net> _nets;
  PtrVector<Bus> _buses;
};

///////////////////////////////////////////////////////////////////////////////////////
//  Instance
class Instance : public Object {
public:
  Instance(const string &name, Module *down_module, Module *owner);

  Module *module() const { return static_cast<Module *>(owner()); }
  Module *down_module() const { return _down_module; }
  const string &module_type() const { return down_module()->type(); }
  bool is_composite() const { return down_module()->is_composite(); }
  void set_down_module(Module *mod); // { _down_module = mod; }

  using pins_type = PtrVector<Pin>::range_type;
  using const_pins_type = PtrVector<Pin>::const_range_type;
  using pin_iter = PtrVector<Pin>::iterator;
  using const_pin_iter = PtrVector<Pin>::const_iterator;

  Pin *create_pin(Pin *mpin) {
    return _pins.add(CosFactory::instance().make_pin(mpin, this));
  }
  Pin *find_pin(const string &name) { return pins().find(name); }
  const Pin *find_pin(const string &name) const { return pins().find(name); }

  std::size_t num_pins() const { return _pins.size(); }
  Pin *pin(int i) { return _pins[i]; }
  const Pin *pin(int i) const { return _pins[i]; }
  pins_type pins() { return _pins.range(); }
  const_pins_type pins() const { return _pins.range(); }
  pins_type pins(const Port *port);
  const_pins_type pins(const Port *port) const;

  Instance *clone(Module *new_owmer) const {
    return static_cast<Instance *>(Object::clone(new_owmer));
  }

  vector<Instance *> fanin_instances() const;
  vector<Instance *> fanout_instances() const;
  void dissolve();
  bool has_input() const;
  void release();

  PropRepository::configs_type configs() const {
    return get_configs(module_type());
  }
  PropertyBase *find_property(const string &name) const {
    Config *cfg = find_config(module_type(), name);
    return cfg ? cfg : Object::find_property(name);
  }

  string path_name() const { return module()->path_name() + "/inst:" + name(); }

protected:
  Object *do_clone(Object *new_owmer) const {
    return static_cast<Module *>(new_owmer)->create_instance(name(),
                                                             down_module());
  }

private:
  friend class Module;                  // access remove_port_pins
  int pin_from(const Port *port) const; // { return port->_pin_from; }
  void remove_port_pins(Port *port);

  Module *_down_module;
  PtrVector<Pin> _pins;
};

///////////////////////////////////////////////////////////////////////////////////////
//  Port
class Port : public Object {
public:
  Port(const string &name, int msb, int lsb, DirType dir, PortType type,
       bool is_vec, Module *owner, int pin_from);

  int msb() const { return _msb; }
  int lsb() const { return _lsb; }
  int width() const { return _width; }
  bool is_vector() const { return _is_vector; }

  Module *module() const { return static_cast<Module *>(owner()); }
  PortType type() const { return _type; }
  DirType dir() const { return _dir; }

  Module::pins_type mpins() { return module()->pins(this); }
  Module::const_pins_type mpins() const {
    return static_cast<const Module *>(module())->pins(this);
  }
  Pin *mpin(int idx = 0) { return module()->pin(_pin_from + idx); }
  const Pin *mpin(int idx = 0) const { return module()->pin(_pin_from + idx); }

  Instance::pins_type up_pins(Instance *inst) { return inst->pins(this); }
  Instance::const_pins_type up_pins(const Instance *inst) const {
    return inst->pins(this);
  }
  Pin *up_pin(Instance *inst, int idx = 0) {
    return inst->pin(_pin_from + idx);
  }
  const Pin *up_pin(const Instance *inst, int idx = 0) const {
    return inst->pin(_pin_from + idx);
  }

  Port *clone(Module *new_owmer) const {
    return static_cast<Port *>(Object::clone(new_owmer));
  }

  void rename(const string &name);
  string path_name() const { return module()->path_name() + "/port:" + name(); }
  virtual string pin_name_format() const { return "%s[%d]"; }

protected:
  Object *do_clone(Object *new_owmer) const {
    return is_vector() ? static_cast<Module *>(new_owmer)->create_port(
                             name(), msb(), lsb(), dir(), type())
                       : static_cast<Module *>(new_owmer)->create_port(
                             name(), dir(), type());
  }

private:
  friend class Pin;      // access _pin_from
  friend class Module;   // access _pin_from
  friend class Instance; // access _pin_from

  PortType _type;
  DirType _dir;
  int _msb, _lsb;
  int _width;
  int _pin_from;
  bool _is_vector;
};

///////////////////////////////////////////////////////////////////////////////////////
//  Pin
class Pin : public Object {
public:
  Pin(const string &name, Port *port, Module *owner, int index, int mindex);
  Pin(Pin *mpin, Instance *owner);
  ~Pin();

  Net *net() const { return _net; }
  const Port *port() const { return _port; }
  Port *port() { return _port; }
  bool is_connected() const { return _net; }
  bool is_mpin() const { return !_mpin; }
  bool is_sink() const { return dir() == INPUT || dir() == INOUT; }
  bool is_source() const { return dir() == OUTPUT || dir() == INOUT; }

  int index() const { return _index; }
  int index_within_module() const { return _mindex; }
  int index_within_instance() const { return _mindex; }
  int index_within_port() const { return _mindex - _port->_pin_from; }

  Module *module() const {
    return is_mpin() ? static_cast<Module *>(owner()) : 0;
  }
  Instance *instance() const {
    return is_mpin() ? 0 : static_cast<Instance *>(owner());
  }
  Module *down_module() const { return port()->module(); }
  Pin *down_pin() const { return _mpin; }
  Pin *up_pin(Instance *inst) const {
    return is_mpin() ? inst->pin(_mindex) : 0;
  }

  DirType dir() const { return _dir; }
  PortType type() const { return port()->type(); }

  void connect(Net *net); // connect to a net
  void disconnect();
  void reconnect(Net *net) {
    disconnect();
    connect(net);
  }

  string path_name() const { return owner()->path_name() + "/pin:" + name(); }

private:
  friend Module::port_iter Module::remove_port(Module::port_iter pit);
  friend class Instance;

  Port *_port;
  Pin *_mpin;
  DirType _dir;
  int _index, _mindex;
  Net *_net;
};

///////////////////////////////////////////////////////////////////////////////////////
//  Net
class Net : public Object {
public:
  Net(const string &name, NetType type, Module *owner, Bus *bus);
  ~Net();

  Module *module() const { return static_cast<Module *>(owner()); }
  Bus *bus() const { return _bus; }
  NetType type() const { return _type; }
  void set_type(NetType type) { _type = type; }

  using pins_type = PtrVector<Pin, false>::range_type;
  using const_pins_type = PtrVector<Pin, false>::const_range_type;
  using pin_iter = PtrVector<Pin, false>::iterator;
  using const_pin_iter = PtrVector<Pin, false>::const_iterator;

  Pin *find_pin(const string &name) { return pins().find(name); }
  const Pin *find_pin(const string &name) const { return pins().find(name); }
  std::size_t num_pins() const { return _pins.size(); }
  pins_type pins() { return _pins.range(); }
  const_pins_type pins() const { return _pins.range(); }
  pins_type source_pins();
  const_pins_type source_pins() const;
  pins_type sink_pins();
  const_pins_type sink_pins() const;

  Net *clone(Module *new_owmer) const {
    return static_cast<Net *>(Object::clone(new_owmer));
  }

  void merge(Net *from);
  void release();
  string path_name() const { return module()->path_name() + "/net:" + name(); }

protected:
  Object *do_clone(Object *new_owmer) const {
    return static_cast<Module *>(new_owmer)->create_net(name(), type());
  }

private:
  void add_pin(Pin *pin);
  void remove_pin(Pin *pin) { _pins.erase(boost::range::find(pins(), pin)); }

  friend void Pin::connect(Net *net);
  friend void Pin::disconnect();

  NetType _type;
  Bus *_bus;

  PtrVector<Pin, false> _pins;
};

///////////////////////////////////////////////////////////////////////////////////////
//  Bus
class Bus : public Object {
public:
  Bus(const string &name, Module *owner, int msb, int lsb);
  Module *module() const { return static_cast<Module *>(owner()); }

  int msb() const { return _msb; }
  int lsb() const { return _lsb; }

  using nets_type = PtrVector<Net, false>::range_type;
  using const_nets_type = PtrVector<Net, false>::const_range_type;
  using net_iter = PtrVector<Net, false>::iterator;
  using const_net_iter = PtrVector<Net, false>::const_iterator;

  std::size_t width() const { return _nets.size(); }
  nets_type nets() { return _nets.range(); }
  const_nets_type nets() const { return _nets.range(); }

  void connect(Port *port,
               ConnectDir dir = CONN_LEFT); // connect to module pins
  void connect(Instance *inst, Port *port,
               ConnectDir dir = CONN_LEFT); // connect to instance pins

private:
  friend Bus *Module::create_bus(const string &name, int msb, int lsb,
                                 NetType type);
  int _msb, _lsb;
  PtrVector<Net, false> _nets;
};

///////////////////////////////////////////////////////////////////////////////////////
//  inline functions

inline Library *Design::work_lib() const { return _top ? _top->library() : 0; }

inline Instance *Module::create_instance(const string &inst_name,
                                         Module *down_module) {
  Instance *inst = _instances.add(
      CosFactory::instance().make_instance(inst_name, down_module, this));
  down_module->_up_instances.add(inst);
  return inst;
}
inline std::pair<Instance *, Module::inst_iter>
Module::create_instance(const string &inst_name, Module *down_module,
                        inst_iter iit) {
  std::pair<Instance *, Module::inst_iter> p = _instances.insert(
      iit, CosFactory::instance().make_instance(inst_name, down_module, this));
  down_module->_up_instances.add(p.first);
  return p;
}

inline Module::inst_iter Module::remove_instance(Module::inst_iter iit) {
  Instance *inst = *iit;
  inst->down_module()->_up_instances.erase(inst);
  return _instances.erase(iit);
}
inline void Module::remove_instance(Instance *inst) {
  inst->down_module()->_up_instances.erase(inst);
  _instances.erase(inst);
}

inline int Module::pin_from(const Port *port) const {
  ASSERTD(port->module() == this, "port belongs to a different module.");
  //		if (port->module() != this) return -1;		// for Release
  return port->_pin_from;
}

inline int Instance::pin_from(const Port *port) const {
  ASSERTD(port->module() == down_module(),
          "port belongs to a different module.");
  //		if (port->module() != down_module()) return -1;		// for
  // Release
  return port->_pin_from;
}

inline void Pin::connect(Net *net) {
  ASSERT(!_net, name() + " : already hooked");
  (_net = net)->add_pin(this);
}
inline void Pin::disconnect() {
  if (_net)
    _net->remove_pin(this), _net = 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//  stream io

std::istream &operator>>(std::istream &s, DirType &);
std::ostream &operator<<(std::ostream &s, DirType);

std::istream &operator>>(std::istream &s, PortType &);
std::ostream &operator<<(std::ostream &s, PortType);

std::istream &operator>>(std::istream &s, ConnectDir &);
std::ostream &operator<<(std::ostream &s, ConnectDir);

} // namespace COS

#endif
