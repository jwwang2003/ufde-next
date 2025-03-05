#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>

#include "io/fileio.hpp"
#include "netlist.hpp"

namespace {
using boost::format;
using boost::adaptors::filtered;
using std::ostream;
using namespace COS;

typedef std::pair<Property<string> *, const char *> init_pair;
init_pair init_props[] = {
    init_pair(&create_temp_property<string>(INSTANCE, "F#INIT_HEX"), "f.INIT"),
    init_pair(&create_temp_property<string>(INSTANCE, "G#INIT_HEX"), "g.INIT"),
    init_pair(&create_temp_property<string>(INSTANCE, "U#INIT_HEX"), "u.INIT"),
    init_pair(&create_temp_property<string>(INSTANCE, "V#INIT_HEX"), "v.INIT"),
    init_pair(0, 0)};
Property<string> VNAME;

const char *valid_chars =
    "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
inline string rename(const string &name) {
  return name.find_first_not_of(valid_chars) == string::npos
             ? name
             : '\\' + name + ' ';
}

void write_cfg(const string &inst_name, const string &cfg_name,
               const string &cfg_value, ostream &os) {
  os << format("\n  defparam %s.%s = \"%s\";") % rename(inst_name) % cfg_name %
            cfg_value;
}

void write_property(const Instance *inst, ostream &os) {
  string inst_name = inst->name() + "_inst";
  for (Config *cfg : inst->configs()) {
    if (!inst->property_exist(*cfg))
      continue;
    string pname = cfg->name();
    if (pname.find("#LUT") != string::npos)
      continue;
    string pvalue = inst->property_value(*cfg);
    if (pname == "SYNC_ATTR") {
      write_cfg(inst_name, "ffx.SYNC_ATTR", pvalue, os);
      write_cfg(inst_name, "ffy.SYNC_ATTR", pvalue, os);
    } else if (pname == "INITX" || pname == "FFX_SR_ATTR") {
      write_cfg(inst_name, "ffx.INIT_VALUE", pvalue, os);
    } else if (pname == "INITY" || pname == "FFY_SR_ATTR") {
      write_cfg(inst_name, "ffy.INIT_VALUE", pvalue, os);
    } else if (pname == "FFX") {
      write_cfg(inst_name, "ffx.TYPE", pvalue, os);
    } else if (pname == "FFY") {
      write_cfg(inst_name, "ffy.TYPE", pvalue, os);
    } else {
      write_cfg(inst_name, boost::to_lower_copy(pname) + ".CONF", pvalue, os);
    }
  }
  for (init_pair *pp = init_props; pp->first; pp++) {
    Property<string> &prop = *pp->first;
    if (!inst->property_exist(prop))
      continue;
    string pname = pp->second;
    string pvalue = inst->property_value(prop);
    if (pvalue[0] == '#')
      pvalue = pvalue[1] == '1' ? "FFFF" : "0000";
    os << format("\n  defparam %s.%s = 16'h%s;") % rename(inst_name) % pname %
              pvalue;
  }
}

void write_instance(const Instance *inst, ostream &os) {
  //		if (inst.cell_type() == "SLICE" || inst.cell_type() == "IOB")
  write_property(inst, os);
  string inst_name = inst->name() + "_inst";
  os << "\n  " << inst->module_type() << ' ' << rename(inst_name);

  const char *sep = " (\n    ";
  for (const Pin *pin : inst->pins()) {
    Net *net = pin->net();
    string net_name = net ? net->property_value(VNAME) : "";
    os << format("%s.%s(%s)") % sep % rename(pin->name()) % net_name;
    sep = ",\n    ";
  }
  os << "\n  );\n";
}

void write_module(const Module *mod, ostream &os) {
  //		if (!IO::Writer::is_used(mod)) return;

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
      if (nports)
        for (const Pin *pin : net->pins() | filtered(is_mpin)) {
          if (pin->is_source()) {
            str_assign +=
                (format("  assign %s = %s;\n") % vname % rename(pin->name()))
                    .str();
          }
          else {
            str_assign +=
                (format("  assign %s = %s;\n") % rename(pin->name()) % vname)
                    .str();
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
  for (const Module *mod : lib->modules())
    write_module(mod, os);
}

} // namespace

namespace COS {
namespace IO {

class SimVerilogWriter : public Writer {
public:
  SimVerilogWriter() : Writer("pack_sim_verilog") {}
  void write(std::ostream &ostrm) const;
};

void SimVerilogWriter::write(std::ostream &ostrm) const {
  //		mark_used();
  write_module(design()->top_module(), ostrm);
}

static SimVerilogWriter simvl_writer; // register writer
bool using_simvl_writer;

} // namespace IO
} // namespace COS
