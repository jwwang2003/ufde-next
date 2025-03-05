#include "netlist.hpp"
#include "io/fileio.hpp"
#include <boost/format.hpp>
#include <locale>
#include <memory>

namespace COS {
using boost::format;
using std::ostream;
using std::vector;

///////////////////////////////////////////////////////////////////////////////////////
//  Property Repository

void Object::copy_property(const Object *from) const {
  for (PropertyBase *p : from->properties())
    if (p->store_copy())
      p->copy(this, from);
}

///////////////////////////////////////////////////////////////////////////////////////
//  Design IO
template <typename T> void load_(Design *self, const string &type, T &src) {
  if (IO::Loader *loader = IO::get_loader(type, self))
    loader->load(src);
}
void Design::save(const string &type, std::ostream &ost) {
  if (IO::Writer *writer = IO::get_writer(type, this))
    writer->write(ost);
}
void Design::save(const string &type, const string &file, bool encrypt) {
  if (IO::Writer *writer = IO::get_writer(type, this))
    writer->write(file, encrypt);
}

void Design::load(const string &type, std::istream &src) {
  load_(this, type, src);
}
void Design::load(const string &type, const std::string &src) {
  load_(this, type, src);
}

///////////////////////////////////////////////////////////////////////////////////////
//  constructors

static inline DirType reverse_dir(DirType dir) {
  return dir == INPUT ? OUTPUT : dir == OUTPUT ? INPUT : dir;
}

Module::Module(const string &name, const string &type, Library *owner)
    : Object(name, MODULE, owner), _type(type) {}

Instance::Instance(const string &name, Module *down_module, Module *owner)
    : Object(name, INSTANCE, owner), _down_module(down_module) {
  for (Pin *pin : down_module->pins())
    create_pin(pin);
}

Port::Port(const string &name, int msb, int lsb, DirType dir, PortType type,
           bool is_vec, Module *owner, int pin_from)
    : Object(name, PORT, owner), _type(type), _dir(dir), _msb(msb), _lsb(lsb),
      _is_vector(is_vec || msb != lsb),
      _width(_msb > _lsb ? _msb - _lsb + 1 : _lsb - _msb + 1),
      _pin_from(pin_from) {}

Pin::Pin(const string &name, Port *port, Module *owner, int index, int mindex)
    : Object(name, PIN, owner), _port(port), _mpin(0),
      _dir(reverse_dir(port->dir())), _net(0), _index(index), _mindex(mindex) {}
Pin::Pin(Pin *mpin, Instance *owner)
    : Object(mpin->name(), PIN, owner), _port(mpin->port()), _mpin(mpin),
      _dir(mpin->port()->dir()), _net(0), _index(mpin->index()),
      _mindex(mpin->index_within_module()) {}
Pin::~Pin() { disconnect(); }

Net::Net(const string &name, NetType type, Module *owner, Bus *bus)
    : Object(name, NET, owner), _type(type), _bus(bus) {}
Net::~Net() {
  while (!_pins.empty())
    _pins.back()->disconnect();
}

Bus::Bus(const string &name, Module *owner, int msb, int lsb)
    : Object(name, BUS, owner), _msb(msb), _lsb(lsb) {}

///////////////////////////////////////////////////////////////////////////////////////
//  Factory
std::unique_ptr<CosFactory> CosFactory::_instance(new CosFactory());
Library *CosFactory::make_library(const string &name, Design *owner,
                                  bool extrn) {
  return new Library(name, owner, extrn);
}
Module *CosFactory::make_module(const string &name, const string &type,
                                Library *owner) {
  return new Module(name, type, owner);
}
Instance *CosFactory::make_instance(const string &name, Module *down_module,
                                    Module *owner) {
  return new Instance(name, down_module, owner);
}
Net *CosFactory::make_net(const string &name, NetType type, Module *owner,
                          Bus *bus) {
  return new Net(name, type, owner, bus);
}
Bus *CosFactory::make_bus(const string &name, Module *owner, int msb, int lsb) {
  return new Bus(name, owner, msb, lsb);
}
Port *CosFactory::make_port(const string &name, int msb, int lsb, DirType dir,
                            PortType type, bool is_vec, Module *owner,
                            int pin_from) {
  return new Port(name, msb, lsb, dir, type, is_vec, owner, pin_from);
}
Pin *CosFactory::make_pin(const string &name, Port *port, Module *owner,
                          int index, int mindex) {
  return new Pin(name, port, owner, index, mindex);
}
Pin *CosFactory::make_pin(Pin *mpin, Instance *owner) {
  return new Pin(mpin, owner);
}

///////////////////////////////////////////////////////////////////////////////////////
//  streaming io for lexical_cast

static const char *pin_dirs[] = {"", "input", "output", "inout"};
static EnumStringMap<DirType> dirmap(pin_dirs);
std::istream &operator>>(std::istream &s, DirType &dir) {
  dir = dirmap.readEnum(s);
  return s;
}
std::ostream &operator<<(std::ostream &s, DirType dir) {
  return dirmap.writeEnum(s, dir);
}

static const char *port_types[] = {"normal", "clock", "reset", "enable",
                                   "carry",  "vcc",   "tbuf",  ""};
static EnumStringMap<PortType> netmap(port_types);
std::istream &operator>>(std::istream &s, PortType &type) {
  type = netmap.readEnum(s);
  return s;
}
std::ostream &operator<<(std::ostream &s, PortType type) {
  return netmap.writeEnum(s, type);
}

static const char *conn_dirs[] = {"left", "right", ""};
static EnumStringMap<ConnectDir> cdirmap(pin_dirs);
std::istream &operator>>(std::istream &s, ConnectDir &dir) {
  dir = cdirmap.readEnum(s);
  return s;
}
std::ostream &operator<<(std::ostream &s, ConnectDir dir) {
  return cdirmap.writeEnum(s, dir);
}

///////////////////////////////////////////////////////////////////////////////////////

template <typename Rng> Rng sub_range(Rng rng, int i, int n = -1) {
  typename Rng::iterator p, q;
  if (i < 0)
    p = q = rng.end();
  else {
    p = rng.begin();
    std::advance(p, i);
    if (n < 0)
      q = rng.end();
    else {
      q = p;
      std::advance(q, n);
    }
  }
  return Rng(p, q);
}

///////////////////////////////////////////////////////////////////////////////////////
//  Module

Port *Module::create_port(const string &name, int msb, int lsb, DirType dir,
                          PortType type) {
  Port *port = _ports.add(CosFactory::instance().make_port(
      name, msb, lsb, dir, type, true, this, _pins.size()));
  int step = msb > lsb ? -1 : 1;
  for (int idx = 0, i = msb; i != lsb + step; i += step, idx++)
    _pins.add(CosFactory::instance().make_pin(
        str(format(port->pin_name_format()) % name % i), port, this, i,
        _pins.size()));
  return port;
}
Port *Module::create_port(const string &name, DirType dir, PortType type) {
  Port *port = _ports.add(CosFactory::instance().make_port(
      name, 0, 0, dir, type, false, this, _pins.size()));
  _pins.add(CosFactory::instance().make_pin(name, port, this, 0, _pins.size()));
  return port;
}

Module::port_iter Module::remove_port(Module::port_iter pit) {
  ASSERTD(pit->module() == this,
          "remove_port: port belongs to a different module.");
  //		ASSERT(num_up_instances() == 0, "remove_port: not supported
  // yet.");

  int w = pit->width();
  pins_type mpins = pit->mpins();
  pin_iter ppin = _pins.erase(mpins.begin(), mpins.end());
  for (; ppin != pins().end(); ++ppin)
    ppin->_mindex -= w;

  for (Instance *inst : up_instances())
    inst->remove_port_pins(*pit);

  pit = _ports.erase(pit);
  for (auto pport = pit; pport != ports().end();
       ++pport) // update _pin_from field for each port
    pport->_pin_from -= w;

  return pit;
}

Module::pins_type Module::pins(const Port *port) {
  return sub_range(pins(), pin_from(port), port->width());
}
Module::const_pins_type Module::pins(const Port *port) const {
  return sub_range(pins(), pin_from(port), port->width());
}

Bus *Module::create_bus(const string &name, int msb, int lsb, NetType type) {
  Bus *bus = _buses.add(CosFactory::instance().make_bus(name, this, msb, lsb));
  int step = msb > lsb ? -1 : 1;
  for (int idx = 0, i = msb; i != lsb + step; i += step, idx++)
    bus->_nets.add(_nets.add(CosFactory::instance().make_net(
        str(format("%s[%d]") % name % i), type, this, bus)));
  return bus;
}

Module::bus_iter Module::remove_bus(bus_iter it) {
  for (Net *n : it->nets())
    remove_net(n);
  return _buses.erase(it);
}

void Module::flatten() {
  vector<Instance *> composite;
  do {
    composite.clear();
    for (Instance *inst : instances())
      if (inst->is_composite())
        composite.push_back(inst);
    for (Instance *inst : composite)
      inst->dissolve();
  } while (!composite.empty());
}

Module *Module::clone(const string &new_name, Library *new_owner) {
  Module *new_module = new_owner->create_module(new_name, type());

  for (const Port *port : ports())
    port->clone(new_module);

  for (const Instance *inst : instances())
    inst->clone(new_module);

  // To do: clone buses
  for (const Net *net : nets()) {
    Net *new_net = net->clone(new_module);
    for (const Pin *pin : net->pins()) {
      int pindex = pin->index_within_module();
      Pin *new_pin =
          pin->is_mpin()
              ? new_module->pin(pindex)
              : new_module->find_instance(pin->owner()->name())->pin(pindex);
      new_pin->connect(new_net);
    }
  }

  return new_module;
}

///////////////////////////////////////////////////////////////////////////////////////
//  Instance

bool Instance::has_input() const {
  return find_if(pins(), [](const Pin *p) {
           return p->is_sink() && p->is_connected();
         }) != pins().end();
}

inline string merge_names(const string &s1, const string &s2,
                          const string &sep = "_") {
  return s1.empty() ? s2 : s2.empty() ? s1 : s1 + sep + s2;
}

void Instance::release() {
  for (Pin *pin : pins())
    pin->disconnect();
  module()->remove_instance(this);
}

void Instance::set_down_module(Module *mod) {
  ASSERT(_down_module->num_ports() == mod->num_ports() &&
             _down_module->num_pins() == mod->num_pins(),
         "set_down_module: port mismatch");
  for (int i = 0; i < _down_module->num_pins(); i++)
    ASSERT(_down_module->pin(i)->name() == mod->pin(i)->name(),
           "set_down_module: port mismatch");
  _down_module = mod;
}

Instance::pins_type Instance::pins(const Port *port) {
  return sub_range(pins(), pin_from(port), port->width());
}
Instance::const_pins_type Instance::pins(const Port *port) const {
  return sub_range(pins(), pin_from(port), port->width());
}

void Instance::remove_port_pins(Port *port) {
  int w = port->width();
  pins_type ppins = pins(port);
  for (Pin *pin : ppins)
    if (pin->net()) {
      FDU_LOG(WARN) << "remove connected pin: " << pin->path_name();
      pin->disconnect();
    }
  pin_iter ppin = _pins.erase(ppins.begin(), ppins.end());
  for (; ppin != pins().end(); ++ppin)
    ppin->_mindex -= w;
}

void Instance::dissolve() {
  if (!is_composite())
    return;

  Property<Net *> rel_net(0);

  for (Pin *pin : pins()) { // merge shortcut nets
    if (Net *onet = pin->net()) {
      //				pin->disconnect();
      //				onet->clear_connections();
      if (Net *inet = pin->down_pin()->net()) {
        if (Net *rnet = inet->property_value(rel_net)) {
          rnet->merge(onet);
        }
        else {
          inet->set_property(rel_net, onet);
        }
      }
    }
  }

  for (Net *net : down_module()->nets()) // create new nets
    if (!net->property_value(rel_net)) {
      Net *new_net =
          module()->create_net(merge_names(name(), net->name()), net->type());
      new_net->copy_property(net);
      net->set_property(rel_net, new_net);
    }

  Property<Point> &rloc = create_property<Point>(INSTANCE, "rloc");
  Property<string> &h_set = create_property<string>(INSTANCE, "h_set");
  for (Instance *inst : down_module()->instances()) { // create new instances
    Instance *new_inst = module()->create_instance(
        merge_names(name(), inst->name()), inst->down_module());
    new_inst->copy_property(inst);
    if (new_inst->property_exist(rloc)) {
      string hset = new_inst->property_value(h_set);
      if (hset.empty())
        hset = "h_set";
      new_inst->set_property(h_set, merge_names(name(), hset));
    }

    for (Pin *pin : new_inst->pins())
      if (Net *old_net = inst->find_pin(pin->name())->net()) {
        Net *new_net = old_net->property_value(rel_net);
        pin->connect(new_net);
        for (Pin *opin : new_net->pins()) {
          if (opin->instance() == this)
            pin->copy_property(opin);
        }
      }
  }
  release();
}

vector<Instance *> Instance::fanout_instances() const {
  vector<Instance *> insts;
  for (const Pin *pin : pins()) {
    Net *net = pin->net();
    if (!pin->is_source() || !net)
      continue;
    for (Pin *sink_pin : net->sink_pins())
      if (!sink_pin->is_mpin())
        insts.push_back(static_cast<Instance *>(sink_pin->owner()));
  }
  return insts;
}

vector<Instance *> Instance::fanin_instances() const {
  vector<Instance *> insts;
  for (const Pin *pin : pins()) {
    Net *net = pin->net();
    if (!pin->is_sink() || !net)
      continue;
    for (Pin *source_pin : net->source_pins())
      if (!source_pin->is_mpin())
        insts.push_back(static_cast<Instance *>(source_pin->owner()));
  }
  return insts;
}

///////////////////////////////////////////////////////////////////////////////////////
//  Port

void Port::rename(const string &name) {
  Object::rename(name);

  if (!_msb && !_lsb && !_is_vector) {
    mpin()->rename(name);
    for (Instance *inst : module()->up_instances())
      up_pin(inst)->rename(name);
  } else {
    int step = _msb > _lsb ? -1 : 1;
    for (int idx = 0, i = _msb; idx < width(); i += step, idx++) {
      string pin_name = (format(pin_name_format()) % name % i).str();
      mpin(idx)->rename(name);
      for (Instance *inst : module()->up_instances())
        up_pin(inst, idx)->rename(name);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//  Net

Net::pins_type Net::source_pins() {
  return Net::pins_type(_pins.begin(), find_if(pins(), [](Pin *pin) {
                          return !pin->is_source();
                        }));
}
Net::const_pins_type Net::source_pins() const {
  return Net::const_pins_type(_pins.begin(), find_if(pins(), [](Pin *pin) {
                                return !pin->is_source();
                              }));
}
Net::pins_type Net::sink_pins() {
  return Net::pins_type(
      find_if(pins(), [](Pin *pin) { return pin->is_sink(); }), _pins.end());
}
Net::const_pins_type Net::sink_pins() const {
  return Net::const_pins_type(
      find_if(pins(), [](Pin *pin) { return pin->is_sink(); }), _pins.end());
}

void Net::add_pin(Pin *pin) {
  if (pin->dir() == OUTPUT)
    _pins.insert(_pins.begin(), pin);
  else if (pin->dir() == INPUT)
    _pins.push_back(pin);
  else
    _pins.insert(sink_pins().begin(), pin); // INOUT
}

void Net::merge(Net *from) {
  if (from == this)
    return;
  ASSERTD(module() == from->module(), "merge: different module.");
  while (!from->_pins.empty())
    from->_pins.back()->reconnect(this);
  from->release();
}

void Net::release() {
  while (!_pins.empty())
    _pins.back()->disconnect();
  module()->remove_net(this);
}

///////////////////////////////////////////////////////////////////////////////////////
//  Bus

void Bus::connect(Port *port, ConnectDir dir) { // connect to module pins
  ASSERT(port->module() == module(),
         "bus connect: port belongs to a different module.");
  int pin_begin, pin_end, step, pi;
  if (dir == CONN_LEFT)
    pin_begin = 0, pin_end = port->width(), step = 1;
  else
    pin_begin = port->width() - 1, pin_end = -1, step = -1;
  net_iter nit;
  for (nit = nets().begin(), pi = pin_begin;
       nit != nets().end() && pi != pin_end; ++nit, pi += step)
    port->mpin(pi)->connect(*nit);
}

void Bus::connect(Instance *inst, Port *port,
                  ConnectDir dir) { // connect to instance pins
  ASSERT(port->module() == inst->down_module(),
         "bus connect: port belongs to a different module.");
  int pin_begin, pin_end, step, pi;
  if (dir == CONN_LEFT)
    pin_begin = 0, pin_end = port->width(), step = 1;
  else
    pin_begin = port->width() - 1, pin_end = -1, step = -1;
  net_iter nit;
  for (nit = nets().begin(), pi = pin_begin;
       nit != nets().end() && pi != pin_end; ++nit, pi += step)
    port->up_pin(inst, pi)->connect(*nit);
}

} // namespace COS
