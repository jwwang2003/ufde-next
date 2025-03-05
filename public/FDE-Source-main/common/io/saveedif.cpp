#include "time.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/adaptors.hpp>

#include "cosmos.hpp"

namespace {
using std::endl;
using std::ostream;
using namespace boost::lambda;
using namespace boost::adaptors;
using boost::erase_all;
using boost::format;
using namespace COS;

const string spchar = "\\<>[]{}()./-";
string rename(string name) {

  size_t pos = name.find_first_of(spchar, 0);
  while (pos != string::npos) {
    name.replace(pos, 1, "_");
    pos = name.find_first_of(spchar, pos + 1);
  }
  if (name.find_first_of("_", 0) == 0)
    name = name.substr(1);
  erase_all(name, " ");
  return name;
}

void write_instance(const Instance &inst, ostream &os) {
  size_t pos = inst.name().find_first_of(spchar, 0);
  if (pos != string::npos) {
    os << format("						(instance "
                 "(rename %s \"%s\")\n") %
              rename(inst.name()) % inst.name();
  } else {
    os << format(
              "						(instance %s\n") %
              inst.name();
  }
  os << format("							"
               "(viewRef PRIM (cellRef %s (libraryRef %s )))\n") %
            inst.down_module().name() % inst.down_module().library().name();
  if (!inst.properties().empty()) {
    foreach (const Object::property_pair &_property, inst.properties()) {
      os << format("							"
                   "(property %s (string \"") %
                _property.first;
      const boost::any *prop = &_property.second.first;
      if (const int *p = boost::any_cast<int>(prop)) {
        os << *p;
      } else if (const string *p = boost::any_cast<string>(prop)) {
        os << *p;
      } else if (const double *p = boost::any_cast<double>(prop)) {
        os << *p;
      } else
        AssertD(true, netlist_error("unknown property type"));
      os << "\"))\n";
    }
  }
  os << "						)\n";
}

void write_net(const Net &net, ostream &os) {
  size_t pos = net.name().find_first_of(spchar, 0);
  if (pos != string::npos)
    os << format("						(net (rename "
                 "%s \"%s\")\n") %
              rename(net.name()) % net.name();
  else
    os << format("						(net %s\n") %
              net.name();
  os << "							(joined\n";
  foreach (const Pin &pin, net.pins()) {
    if (!pin.is_mpin() && !pin.port().group())
      os << format("							"
                   "	(portRef %s (instanceRef %s ))\n") %
                rename(pin.name()) % rename(pin.owner().name());
    else if (!pin.is_mpin() && pin.port().group())
      os << format("							"
                   "	(portRef ( member %s %s )(instanceRef %s ))\n") %
                pin.port().group()->name() % pin.port().grp_idx() %
                rename(pin.owner().name());
    else if (pin.is_mpin() && pin.port().group())
      os << format("							"
                   "	(portRef ( member %s %s ))\n") %
                pin.port().group()->name() % pin.port().grp_idx();
    else // pin.is_mpin() && !pin.port().group()
      os << format("							"
                   "	(portRef %s )\n") %
                rename(pin.name());
  }
  os << "							)\n"; // for
                                                                      // joined
  os << "						)\n";         // for net
}

void write_module(const Module &cell, ostream &os) {
  if (!cell.property_value(IO::used, false))
    return;

  os << format("			(cell %s (cellType %s )\n") %
            cell.name() % cell.type();
  os << "				(view PRIM (viewType NETLIST)\n";
  os << "					(interface \n";

  foreach (const Port &port, cell.ports()) {
    size_t pos = port.name().find_first_of(spchar, 0);
    if (!port.group() && pos != string::npos)
      os << format("						(port %s "
                   "(Direction %s))\n") %
                rename(port.name()) % port.dir();
    else if (!port.group() && pos == string::npos)
      os << format("						(port %s "
                   "(Direction %s))\n") %
                port.name() % port.dir();
    else if (port.grp_idx() == 0) {
      os << "						(port (array (rename ";
      os << port.group()->name();
      os << " \"" << port.group()->name() << '[' << port.group()->msb() << ':'
         << port.group()->lsb() << "]\") ";
      os << port.group()->msb() - port.group()->lsb() + 1 << " ) ";
      os << format(" (Direction %s))\n") % port.dir();
    }
  }
  os << "					)\n"; // for interface

  // cell instances
  if (cell.is_composite()) {
    os << "					(contents \n";
    for_each(cell.instances(), bind(&write_instance, _1, var(os)));
    for_each(cell.nets(), bind(&write_net, _1, var(os)));
    os << "					)\n"; // for content
  }
  os << "				)\n"; // for view
  os << "			)\n";         // for cell
}

void write_library(const Library &lib, ostream &os) {
  bool is_empty = true;
  foreach (const Module &cell, lib.modules()) {
    if (cell.property_value(IO::used, false)) {
      is_empty = false;
      break;
    }
  }
  if (!is_empty) {
    os << "		(library " << lib.name() << endl;
    os << "			(edifLevel 0)" << endl;
    os << "			(technology (numberDefinition))" << endl;
    for_each(lib.modules(), bind(&write_module, _1, var(os)));
    os << "		)\n"; // for library
  }
}

void write_design(const Design &design, std::ostream &os) {
  Library &work_lib = design.work_lib();
  foreach (const Library &lib, design.libs())
    if (&lib != &work_lib && lib.num_modules())
      write_library(lib, os);
  write_library(work_lib, os);
}

void write_edif_header(const string design_name, const string author,
                       const string program, const string version,
                       ostream &os) {
  struct tm *local; // set the time
  time_t t;
  t = time(nullptr);
  local = localtime(&t);
  os << "(edif " << design_name << endl;
  os << "		(edifVersion 2 0 0)" << endl;
  os << "		(edifLevel 0)" << endl;
  os << "		(keywordMap (keywordLevel 0))" << endl;
  os << "		(status" << endl;
  os << "				(written" << endl;
  os << format("					(timeStamp  %d %d %d "
               "%d %d %d)\n") %
            (local->tm_year + 1900) % (local->tm_mon + 1) % local->tm_mday %
            local->tm_hour % local->tm_min % local->tm_sec;
  os << "					(author \"" << author << "\")"
     << endl;
  os << "					(program \"" << program << "\""
     << "(version \"" << version << "\"))" << endl;
  os << "				)" << endl;
  os << "		)" << endl;
}

void write_terminal_info(const Design &design, ostream &os) {
  os << format("		(design %s (cellRef %s (libraryRef %s)))\n") %
            design.name() % design.top_module().name() %
            design.top_module().library().name();
  os << ")";
}

} // namespace

namespace COS {
namespace IO {

class EdifWriter : public Writer {
public:
  EdifWriter() : Writer("edif") {}
  void write(std::ostream &ostrm, const std::string &lib);
};

void EdifWriter::write(std::ostream &os, const std::string &lib_name) {
  if (lib_name.empty()) {
    mark_used();
    write_edif_header(design().name(), "FUDAN UNIVERSITY", "FDPTMR", "1.0", os);
    write_design(design(), os);
    // 			write_library(*design().libs().find("UNISIMS"), os);
    // 			write_library(design().work_lib(), os);
    write_terminal_info(design(), os);
  } else {
    const Library *lib = design().libs().find(lib_name);
    Assert(lib, netlist_error(lib_name + ": library not found"));
  }
}

static EdifWriter edif_writer; // register writer

} // namespace IO
} // namespace COS