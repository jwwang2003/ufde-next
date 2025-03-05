#include <functional>

#include "io/fileio.hpp"
#include "netlist.hpp"

namespace {
using std::endl;
using std::ostream;
using namespace COS;

void consistency_check(const Net *net, ostream &logger) {
  for (const Pin *pin : net->pins())
    if (pin->net() != net)
      logger << *pin << " : net error.\n";

  size_t num_sources = net->source_pins().size();
  if (num_sources > 1)
    logger << *net << " : multiple sources." << endl;
  if (!num_sources)
    logger << *net << " : no sources." << endl;
  if (net->num_pins() <= num_sources)
    logger << *net << " : no sinks." << endl;
}

void consistency_check(const Port *port, ostream &logger) {
  if (!port->mpin()->is_mpin())
    logger << port << " : mpin error." << endl;
  if (port->mpin()->port() != port)
    logger << port << " : mpin port error." << endl;
  //		if (port->mpin()->name() != port->name()) logger << port << " :
  // mpin name error." << endl;
}

void consistency_check(const Pin *pin, ostream &logger) {
  if (pin->name() != pin->port()->name())
    logger << pin << " : name error." << endl;
  const Net *net = pin->net();
  if (net && find(net->pins(), pin) == net->pins().end())
    logger << pin << " : net error." << endl;
}

void consistency_check(const Instance *inst, ostream &logger);
void consistency_check(const Module *cell, ostream &logger);
void consistency_check(const Library *lib, ostream &logger);
void consistency_check(const Design *design, ostream &logger);

template <typename T, typename O>
inline std::function<void(T)> member_check(const O &owner, ostream &logger) {
  return [&](T t) {
    if (t->owner() != owner)
      logger << t << " : owner error." << endl;
    consistency_check(t, logger);
  };
}

#define member_check_all(owner, members, logger)                               \
  for (const auto *m : owner->members)                                         \
  member_check<decltype(m)>(owner, logger)(m)

void consistency_check(const Instance *inst, ostream &logger) {
  member_check_all(inst, pins(), logger);
  const Module *inst_of = inst->down_module();

  for (const Pin *pin : inst->pins()) {
    const Port *port = inst_of->find_port(pin->name());
    if (!port || port != pin->port())
      logger << *pin << " : port error." << endl;
  }
  for (const Port *port : inst_of->ports()) {
    const Pin *pin = inst->find_pin(port->name());
    if (!pin || port != pin->port())
      logger << *port << " : pin error." << endl;
  }
}

void consistency_check(const Module *cell, ostream &logger) {
  member_check_all(cell, ports(), logger);
  member_check_all(cell, instances(), logger);
  member_check_all(cell, nets(), logger);
}

void consistency_check(const Library *lib, ostream &logger) {
  member_check_all(lib, modules(), logger);
}

void consistency_check(const Design *design, ostream &logger) {
  member_check_all(design, libs(), logger);
}

} // namespace

namespace COS {
namespace IO {

class CheckWriter : public Writer {
public:
  CheckWriter() : Writer("check") {}
  void write(std::ostream &ostrm) const;
};

void CheckWriter::write(std::ostream &ostrm) const {
  consistency_check(design(), ostrm);
}

static CheckWriter chk_writer;
bool using_chk_writer;
} // namespace IO
} // namespace COS
