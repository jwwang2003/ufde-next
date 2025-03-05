#include <boost/format.hpp>

#include "io/fileio.hpp"
#include "netlist.hpp"

namespace {
using boost::format;
using std::ostream;
using namespace COS;

Property<string> &INIT = create_property<string>(INSTANCE, "INIT", "0000");
Property<string> VNAME;

const char *valid_chars =
    "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
inline string rename(const string &name) {
  return name.find_first_not_of(valid_chars) == string::npos
             ? name
             : '\\' + name + ' ';
}

string rename_pin(const Pin *pin) {
  const Port *port = pin->port();
  return port->is_vector()
             ? str(format("%s[%d]") % rename(port->name()) % pin->index())
             : rename(pin->name());
}

void write_init(const string &inst_name, const string &pname,
                const string &pvalue, ostream &os) {
  os << format("\n  defparam %s.%s = %d'h%s;") % rename(inst_name) % pname %
            (pvalue.length() * 4) % pvalue;
}

void write_property(const Instance *inst, ostream &os) {
  for (PropertyBase *prop : inst->properties()) {
    if (!inst->property_exist(*prop))
      continue;
    if (inst->down_module()->type() == "FFLATCH")
      continue; // for yosys
    string pname = prop->name();
    if (pname.find("INIT") != 0)
      continue;
    string pvalue = prop->string_value(inst);
    write_init(inst->name(), pname, pvalue, os);
  }
}

void write_instance(const Instance *inst, ostream &os) {
  write_property(inst, os);
  os << "\n  " << rename(inst->down_module()->name()) << ' '
     << rename(inst->name());

  const char *sep = " (\n    ";
  string grp_names = "{";
  for (const Pin *pin : inst->pins()) {
    Net *net = pin->net();
    string net_name = net ? net->property_value(VNAME) : "";
    const Port *port = pin->port();
    if (!port->is_vector()) { // single pin
      os << format("%s.%s(%s)") % sep % rename_pin(pin) % net_name;
      sep = ",\n    ";
    } else if (pin->index_within_port() < port->width() - 1) { // group
      grp_names += net_name + ", ";
    } else { // last pin in group
      grp_names += net_name + "}";
      os << format("%s.%s(%s)") % sep % rename(port->name()) % grp_names;
      grp_names = "{";
      sep = ",\n    ";
    }
  }
  os << "\n  );\n";
}

void write_module(const Module *mod, ostream &os) {
  string str_ports;
  const char *sep = " (";
  os << "\nmodule " << rename(mod->name());
  for (const Port *port : mod->ports()) {
    os << sep << rename(port->name());
    sep = ", ";
    str_ports += (format(" %s ") % port->dir()).str();
    if (port->is_vector())
      str_ports += (format("[%d:%d] %s;\n") % port->msb() % port->lsb() %
                    rename(port->name()))
                       .str();
    else
      str_ports += rename(port->name()) + ";\n";
  }
  if (mod->num_ports())
    os << ')';
  os << ";\n";

  // module ports
  os << str_ports;

  // module signals
  string str_assign = "\n";
  for (const Net *net : mod->nets()) {
    string vname = net->name();
    auto is_mpin = [](const Pin *pin) { return pin->is_mpin(); };
    auto nports = count_if(net->pins(), is_mpin);
    if (!(nports == 1 && vname == find_if(net->pins(), is_mpin)->name())) {
      vname = rename(vname);
      os << "  wire " << vname << ";\n";
      if (nports) {
        for (const Pin *pin : net->pins()) {
          if (pin->is_mpin()) {
            if (pin->is_source()) {
              str_assign +=
                  str(format("  assign %s = %s;\n") % vname % rename_pin(pin));
            } else {
              str_assign +=
                  str(format("  assign %s = %s;\n") % rename_pin(pin) % vname);
            }
          }
        }
      }
    }
    const_cast<Net *>(net)->set_property(VNAME, vname);
  }

  os << str_assign;

  // module instances
  for (const Instance *inst : mod->instances())
    write_instance(inst, os);
  os << "endmodule\n";
}

void write_library(const Library *lib, ostream &os) {
  for (const Module *module : IO::Writer::used_module(lib))
    write_module(module, os);
}

void write_design(const Design *design, ostream &os) {
  for (const Library *lib : IO::Writer::used_lib(design))
    if (!lib->is_external())
      write_library(lib, os);
}

} // namespace

namespace COS {
namespace IO {

class VerilogWriter : public Writer {
public:
  VerilogWriter() : Writer("verilog") {}
  void write(std::ostream &ostrm) const;
};

void VerilogWriter::write(std::ostream &ostrm) const {
  mark_used();
  write_design(design(), ostrm);
  VNAME.clear();
}

static VerilogWriter vl_writer; // register writer
bool using_vl_writer;

} // namespace IO
} // namespace COS
