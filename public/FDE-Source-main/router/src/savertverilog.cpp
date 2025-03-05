#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <iomanip>
#include <sstream>

#include "RTFactory.h"
#include "RTMsg.h"
#include "RTNode.h"
#include "arch/archlib.hpp"
#include "cillib.h"
#include "io/fileio.hpp"
#include "property.hpp"
#include "rrg/rrg.hpp"
#include "rtnetlist.hpp"
#include <deque>
#include <map>
#include <string>

namespace {
using boost::format;
using boost::adaptors::filtered;
using std::ostream;
using namespace COS;
using namespace ARCH;
using namespace FDU::RRG;
using namespace FDU::RT;
using std::deque;
using std::queue;
using namespace FDU::rt_cil_lib;
using std::map;
using std::multimap;

queue<PIP *> Pipinfo;
queue<RTNode *> Sinknodeinfo;
map<string, Config *> Configinfo;
queue<Pin *> Pininfo;
string _def_val_conf = "#OFF";
multimap<string, Point> interPipinfo;

typedef std::pair<Property<string> *, const char *> init_pair;
init_pair init_props[] = {
    init_pair(&create_temp_property<string>(INSTANCE, "F#INIT_HEX"), "f.INIT"),
    init_pair(&create_temp_property<string>(INSTANCE, "G#INIT_HEX"), "g.INIT"),
    init_pair(&create_temp_property<string>(INSTANCE, "U#INIT_HEX"), "u.INIT"),
    init_pair(&create_temp_property<string>(INSTANCE, "V#INIT_HEX"), "v.INIT"),
    init_pair(0, 0)};

typedef std::tuple<RRGArchNet *, RRGArchNet *, ArchPath *> Path;
typedef PtrVector<Path> Paths;

typedef std::list<RTNode *>::iterator path_node_iter;
typedef std::list<RTNode *>::const_iterator const_path_node_iter;

Property<string> VNAME;
Property<Point> &postion = create_property<Point>(COS::INSTANCE, "position");
Property<string> &cil_filename =
    create_property<string>(COS::DESIGN, "cil_fname");

const char *valid_chars =
    "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
inline string rename(const string &name) {
  return name.find_first_not_of(valid_chars) == string::npos
             ? name
             : '\\' + name + ' ';
}

string rename_port(string &&name) {
  if ((name[0] >= '0' && name[0] <= '9') || name[0] == '_') {
    name = 's' + name;
  }
  return name;
}

void write_cfg(const string &inst_name, const string &cfg_name,
               const string &cfg_value, ostream &os) {
  os << format("\n  defparam %s.%s = \"%s\";") % rename(inst_name) % cfg_name %
            cfg_value;
}

string transform_property(string pvalue) {
  std::stringstream ioss;
  string s_tmp;
  long value = 0, row, column, base;

  const int MAXI = 4;
  static const char *const item[MAXI][(1 << MAXI) + 1] = {
      {"~A1*A1", "A1", "~A1"},
      {"(~A1*A1)+(~A2*A2)", "(A2*A1)", "(A2*~A1)", "(~A2*A1)", "(~A2*~A1)"},
      {"(~A1*A1)+(~A2*A2)+(~A3*A3)", "((A3*A2)*A1)", "((A3*A2)*~A1)",
       "((A3*~A2)*A1)", "((A3*~A2)*~A1)", "((~A3*A2)*A1)", "((~A3*A2)*~A1)",
       "((~A3*~A2)*A1)", "((~A3*~A2)*~A1)"},
      {"(~A1*A1)+(~A2*A2)+(~A3*A3)+(~A4*A4)", "(((A4*A3)*A2)*A1)",
       "(((A4*A3)*A2)*~A1)", "(((A4*A3)*~A2)*A1)", "(((A4*A3)*~A2)*~A1)",
       "(((A4*~A3)*A2)*A1)", "(((A4*~A3)*A2)*~A1)", "(((A4*~A3)*~A2)*A1)",
       "(((A4*~A3)*~A2)*~A1)", "(((~A4*A3)*A2)*A1)", "(((~A4*A3)*A2)*~A1)",
       "(((~A4*A3)*~A2)*A1)", "(((~A4*A3)*~A2)*~A1)", "(((~A4*~A3)*A2)*A1)",
       "(((~A4*~A3)*A2)*~A1)", "(((~A4*~A3)*~A2)*A1)",
       "(((~A4*~A3)*~A2)*~A1)"}};

  if (pvalue != "1") {
    vector<string> vStr;
    boost::split(vStr, pvalue, boost::is_any_of("+"), boost::token_compress_on);
    for (vector<string>::iterator it = vStr.begin(); it != vStr.end(); it++) {

      s_tmp = *it;

      for (row = 0, base = 2; row < 4; base *= 2, row++) {
        for (column = 0; column < base + 1; column++) {
          if (item[row][column] == s_tmp && column != 0)
            value += 1 << (base - column);
        }
      }
    }
    ioss << std::resetiosflags(std::ios::uppercase) << std::hex << value;
    ioss >> s_tmp;
  } else {
    s_tmp = "ffff";
  }
  return s_tmp;
}

Property<string> &name_transform(string name) {
  Property<string> *result = nullptr;
  for (init_pair *pp = init_props; pp->first; pp++) {
    string pname = pp->second;

    if (pname.substr(0, 1) == boost::to_lower_copy(name)) {
      result = pp->first;
    }
  }

  if (result == nullptr) {
    throw std::runtime_error("name_transform error");
  }

  return *result;
}

void write_property(Instance *inst, ostream &os) {
  string inst_name = inst->name() + "_inst";
  for (Config *cfg : inst->configs()) {
    if (inst->property_value(*cfg) == _def_val_conf) {
      continue;
    }
    string pname = cfg->name();
    if (pname.find("#LUT") != string::npos)
      continue;
    string pvalue = inst->property_value(*cfg);
    if (pvalue.substr(0, 7) == "#LUT:D=") {
      // std::cout << pvalue.substr(7) << "\n";
      inst->set_property(name_transform(pname),
                         transform_property(pvalue.substr(7)));
    }
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

void set_gsb_default(Instance *inst, elemLib *elemlib) {
  Element *ele;
  string _default;
  map<string, Config *>::iterator iter;
  Config *conf;

  _default = "";
  for (Instance *inst_in_down_module : inst->down_module()->instances()) {
    iter = Configinfo.find(inst_in_down_module->name());
    if (iter != Configinfo.end()) {
      conf = iter->second;
    } else {
      conf = &create_config("GSB", inst_in_down_module->name());
      Configinfo[inst_in_down_module->name()] = conf;
    }
    ele = elemlib->getElement(inst_in_down_module->down_module()->name());
    for (sramElem &sram : ele->getSrams().srams()) {
      if (sram.getdefault())
        _default += boost::lexical_cast<string>(sram.getContent());
    }
    if (_default != "")
      inst->set_property(*conf, _default);
    _default = "";
  }
}

void add_config(Instance *inst, string from, string to, elemLib *elementlib) {

  Module *mod = inst->down_module();
  // Net *net_to, *net_from;
  string port_to, port_from;
  RRGArchInstance *instanceref = nullptr;
  Element *ele;
  string config;
  map<string, Config *>::iterator iter;
  for (RRGArchInstance *ainst :
       static_cast<RRGArchCell *>(inst->down_module())->instances()) {
    for (Path *path : ainst->paths()) {
      if (std::get<0>(*path)->name() == from &&
          std::get<1>(*path)->name() == to) {
        instanceref = ainst;
        port_from = std::get<2>(*path)->from_port()->name();
        port_to = std::get<2>(*path)->to_port()->name();
      }
    }
  }
  config = "";
  if (instanceref) {
    ele = elementlib->getElement(instanceref->down_module()->name());
    for (pathElem &path : ele->getPaths().paths()) {
      if (path.getIn() == port_from && path.getOut() == port_to) {
        for (sramElem &sram : path.getCfgInfo().srams()) {
          config += boost::lexical_cast<string>(sram.getContent());
        }
      }
    }
  }

  if (config != "") {
    iter = Configinfo.find(instanceref->name());
    assert(iter != Configinfo.end());
    inst->set_property(*(iter->second), config);
  }
}

Instance *find_gsb_down_mod(FPGADesign *fpgadesign, Point pos) {
  Instance *ins_tmp = fpgadesign->get_inst_by_pos(pos);
  Module *mod_tmp = ins_tmp->down_module();
  for (Instance *inst : mod_tmp->instances()) {
    if (inst->module_type() == "GSB") {
      return inst;
    }
  }
  return nullptr;
}

Pin *find_port_pin(FPGADesign *fpgadesign, Instance *gsb_inst, string name,
                   Point pos) {
  Pin *return_pin;
  Instance *ins_tmp = fpgadesign->get_inst_by_pos(pos);
  Module *mod_tmp = ins_tmp->down_module();
  Net *net_tmp;

  if ((net_tmp = mod_tmp->find_net(name)) != nullptr) {
    for (Pin *pin_tmp : net_tmp->pins()) {
      if (pin_tmp->instance() != nullptr)
        if (pin_tmp->instance()->down_module()->type() == "GSB") {
          return_pin = gsb_inst->find_pin(pin_tmp->name());
        }
    }
  } else {
    return_pin = gsb_inst->find_pin(name);
  }
  return return_pin;
}

void connect_two_pins(Pin *pin_from, Pin *pin_to, string net_name, RTCell *mod,
                      Point pos) {
  Net *new_net, *net_old;

  string new_net_name = net_name + "_" + pin_from->name() + "_to_" +
                        pin_to->name() +
                        boost::lexical_cast<string>(pos.x + 1) + "_" +
                        boost::lexical_cast<string>(pos.y + 1) + "_" +
                        boost::lexical_cast<string>(pos.z + 1);

  if (pin_to->net()) {
    return;
  }
  if ((net_old = pin_from->net()) != 0) {
    pin_to->connect(net_old);
  } else {
    new_net = mod->create_net(new_net_name);
    new_net->set_property(VNAME, new_net_name);
    pin_to->connect(new_net);
    pin_from->connect(new_net);
  }
}

void add_gsb(FPGADesign *fpgadesign, RTCell *rtmod, string cil_fname,
             ostream &os) {

  vector<RTNet *> rtnets;

  // ���ڼ�¼��Ҫ���ӵ� GSB Instance
  Instance *gsbInst;
  map<Point, Instance *> gsbInstInfos;
  map<Point, Instance *>::iterator gsbIter;

  // ���� cil ���ļ����������� GSB ������
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_LOAD_ARCH) % cil_fname; // arch�ļ�
  cilLibrary *cillib = cilLibrary::loadCilLib(cil_fname);
  elemLib *elementlib = cillib->getElemLib();

  // �����Ѳ��� net�� �����Ӳ�ͬλ�õ� GSB instance
  for (RTNet *rtnet : rtmod->nets())
    rtnets.push_back(rtnet);
  for (RTNet *rtnet : rtnets) {

    // ����ĳЩ�����ߣ�clk �ȣ�������û�� pip ��
    if (rtnet->num_pips() == 0)
      continue;

    RRGSwitch *p_sw;
    path_node_iter from_node_it, to_node_it;
    path_node_iter end_node = rtnet->path_end();
    Pin *prev_pin = nullptr;

    if (rtnet->num_pips() == 0)
      continue;

    // ��¼ src_pin �Լ� sink_pins. �� rtnet ����� pin �����ӹ�ϵ�Ͽ�
    Pin *src_pin = *(boost::find_if(
        rtnet->pins(), [](const Pin *pin) { return pin->is_source(); }));
    vector<Pin *> sink_pins;
    src_pin->disconnect();
    for (Pin *pin : rtnet->sink_pins()) {
      sink_pins.push_back(pin);
    }
    for (Pin *pin : sink_pins)
      pin->disconnect();

    prev_pin = src_pin;
    map<RTNode *, Pin *> visitedNodesMap;
    map<RTNode *, Pin *>::iterator visitedIter;
    for (from_node_it = rtnet->path_begin(); from_node_it != end_node;
         ++from_node_it) {
      // ���ҵ� pip ���ڵ� position �Ƿ��ж�Ӧ GSB
      if ((*from_node_it)->is_sink())
        continue;
      to_node_it = from_node_it;
      ++to_node_it;

      p_sw = (*from_node_it)->find_switch(*to_node_it);
      ASSERT(p_sw != nullptr, ErrMsg(ErrMsg::RTERR_UFND_SWH) %
                               (*from_node_it)->full_name() %
                               (*to_node_it)->full_name());

      Instance *gsb_down_inst = find_gsb_down_mod(fpgadesign, p_sw->pos());
      Module *gsb_down_mod = gsb_down_inst->down_module();
      string gsb_inst_name =
          gsb_down_mod->name() + "_" +
          boost::lexical_cast<string>(p_sw->pos().x + 1) + "_" +
          boost::lexical_cast<string>(p_sw->pos().y + 1) + "_" +
          boost::lexical_cast<string>(p_sw->pos().z + 1);
      gsbIter = gsbInstInfos.find(p_sw->pos());
      if (gsbIter != gsbInstInfos.end()) {
        gsbInst = gsbIter->second;
      } else {
        // ���Ҳ����򴴽�һ�� GSB Instance
        gsbInst = rtmod->create_instance(gsb_inst_name, gsb_down_mod);
        set_gsb_default(gsbInst, elementlib);
        gsbInstInfos[p_sw->pos()] = gsbInst;
      }
      // �� GSB ���� config �ļ�
      add_config(gsbInst, p_sw->from_net()->name(), p_sw->to_net()->name(),
                 elementlib);

      Net::const_pin_iter from_up_pin_iter =
          boost::find_if(p_sw->from_net()->pins(),
                         [](const Pin *pin) { return pin->is_mpin(); });
      Pin *from_up_pin = (from_up_pin_iter != p_sw->from_net()->pins().end()
                              ? (*from_up_pin_iter)->up_pin(gsbInst)
                              : nullptr);

      Net::const_pin_iter to_up_pin_iter =
          boost::find_if(p_sw->to_net()->pins(),
                         [](const Pin *pin) { return pin->is_mpin(); });

      Pin *to_up_pin = (to_up_pin_iter != p_sw->to_net()->pins().end()
                            ? (*to_up_pin_iter)->up_pin(gsbInst)
                            : nullptr);
      Net *to_mod_net = (to_up_pin_iter != p_sw->to_net()->pins().end()
                             ? (*to_up_pin_iter)->up_pin(gsb_down_inst)->net()
                             : nullptr);

      // ��� to_node û�з��ʹ�������� visitedNodesMap
      visitedIter = visitedNodesMap.find((*to_node_it));
      if (visitedIter == visitedNodesMap.end())
        visitedNodesMap[*(to_node_it)] = to_up_pin;

      visitedIter = visitedNodesMap.find((*from_node_it));
      if (from_up_pin && from_up_pin != prev_pin) {
        // ��� from_node �Ѿ����ʹ�
        if (visitedIter != visitedNodesMap.end()) {
          prev_pin = visitedIter->second;
          if (prev_pin != from_up_pin)
            connect_two_pins(prev_pin, from_up_pin, rtnet->name(), rtmod,
                             p_sw->pos());
        } else {
          connect_two_pins(prev_pin, from_up_pin, rtnet->name(), rtmod,
                           p_sw->pos());
        }
      }

      // ���� to_node_it ���˵� pin
      if ((*to_node_it)->is_sink()) {
        string slicePinName;
        ASSERT(to_mod_net->num_pins() == 2,
               ErrMsg(ErrMsg::ERROR_NUM_PINS) % to_mod_net->name());

        for (Pin *pin : to_mod_net->pins()) {
          if (pin->name() != to_up_pin->name())
            slicePinName = pin->name();
        }
        vector<Pin *>::iterator sink_pin = sink_pins.end();
        sink_pin = std::find_if(
            sink_pins.begin(), sink_pins.end(),
            [slicePinName](const Pin *a) { return a->name() == slicePinName; });
        if (sink_pin == sink_pins.end() && src_pin->name() == "COUT")
          sink_pin =
              std::find_if(sink_pins.begin(), sink_pins.end(),
                           [](const Pin *a) { return a->name() == "CIN"; });
        ASSERT(sink_pin != sink_pins.end(),
               ErrMsg(ErrMsg::ERROR_FIND_SINK_PIN) % to_up_pin->name() %
                   to_mod_net->name());
        connect_two_pins(to_up_pin, *sink_pin, rtnet->name(), rtmod,
                         p_sw->pos());
        prev_pin = src_pin;
        sink_pins.erase(sink_pin);
      } else {
        if (to_up_pin)
          prev_pin = to_up_pin;
      }
    }
  }

  // ��֮ǰ�� net ���Ƴ�
  for (RTNet *rtnet : rtnets) {
    if (rtnet->num_pips() == 0)
      continue;
    rtmod->remove_net(rtnet);
  }
}

void write_instance(Instance *inst, ostream &os) {
  write_property(inst, os);
  string inst_name = inst->name() + "_inst";
  if (inst->module_type() != "GSB")
    os << "\n  " << inst->module_type() << ' ' << rename(inst_name);
  else
    os << "\n  " << inst->down_module()->name() << ' ' << rename(inst_name);

  const char *sep = " (\n    ";
  for (const Pin *pin : inst->pins()) {
    Net *net = pin->net();
    string net_name = net ? net->property_value(VNAME) : "";
    os << format("%s.%s(%s)") % sep % rename_port(rename(pin->name())) %
              net_name;
    sep = ",\n    ";
  }
  os << "\n  );\n";
}

void write_module(FPGADesign *fpgadesign, Module *mod, string cil_fname,
                  ostream &os) {

  string str_ports;
  Router *router_ = &Router::Instance();

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

  // add gsb
  add_gsb(fpgadesign, router_->get_rtcell(), cil_fname, os);

  // module signals
  string str_assign = "\n";
  auto is_mpin = [](const Pin *pin) { return pin->is_mpin(); };
  for (const Net *net : mod->nets()) {
    string vname = net->name();
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
  for (Instance *inst : mod->instances())
    write_instance(inst, os);
  // for (const GSB* gsb : gsb_iters()) write_GSB_instance(gsb,os);
  os << "endmodule\n";
}

void write_cfg_test(Module *mod, const Design *fpgadesign, ostream &os) {
  Library *prim_lib = FPGADesign::instance()->find_library("primitive");
  typedef std::tuple<RRGArchNet *, RRGArchNet *, ArchPath *> Path;
  typedef PtrVector<Path> Paths;

  if (!prim_lib) {
    prim_lib = FPGADesign::instance()->find_library("block");
  }
  for (Module *prim_cell : prim_lib->modules()) {
    if (prim_cell->type() != "GRM" && prim_cell->type() != "GSB")
      continue;
    // for all instances in GRM
    for (RRGArchInstance *ainst :
         static_cast<RRGArchCell *>(prim_cell)->instances()) {
      for (Path *path : ainst->paths()) {
        os << std::get<0>(*path)->name() << ":" << std::get<1>(*path)->name()
           << ainst->name() << ":"
           << "\n";
      }
    }
  }
}

} // namespace

namespace COS {
namespace IO {

class RTVerilogWriter : public Writer {
public:
  RTVerilogWriter() : Writer("route_sim_verilog") {}
  void write(std::ostream &ostrm) const;
};

void RTVerilogWriter::write(std::ostream &ostrm) const {
  FPGADesign *fpgadesign = ARCH::FPGADesign::instance();
  write_module(fpgadesign, design()->top_module(),
               design()->property_value(cil_filename), ostrm);

  // write_cfg_test(design()->top_module(), fpgadesign,ostrm);
}

static RTVerilogWriter rtvl_writer; // register writer
bool using_rtsimvl_writer;
} // namespace IO
} // namespace COS
