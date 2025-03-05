#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <exception>
#include <vector>

#ifdef _DEBUG
#include <time.h>
#endif

#include "NLFiner.hpp"
#include "property.hpp"

using namespace std;
using namespace COS;
using namespace boost;

namespace FDU {
namespace NLF {

const string PRIMITIVE_LIB = "template_cell_lib";
const string BLOCK_LIB = "template_work_lib";
const string VCC_OUT = "P";
const string GND_OUT = "G";

void NLFiner::refine() {
  rebuild_verbose_netlist();
  for (Instance *inst : _design->top_module()->instances())
    refine_instance(inst);
}

void NLFiner::rebuild_verbose_netlist() {
  // old netlist, do not need to refine
  if (Library *design_prim_lib = _design->find_library(PRIMITIVE_LIB))
    return;
  copy_template_cell_lib();
  map<string, size_t> name_repo; // size_t will be initialized to 0 when the key
                                 // is first visited
  for (Instance *inst : _design->top_module()->instances())
    rebuild_verbose_instance(inst, name_repo);

  remove_unused_module();
#ifdef _DEBUG
  // 		for (Library* lib: _design->libs())
  // 		{
  // 			FDU_LOG(DEBUG) << "LIB  ==========  " + lib->name();
  // 			for (Module* mod: lib->modules()) {
  // 				FDU_LOG(DEBUG) << "Module: " + mod->name();
  // 				for (Instance* inst: mod->instances()) {
  // 					FDU_LOG(DEBUG) << "Instance: " +
  // inst->name();
  // 				}
  // 			}
  // 		}
  _design->save("xml", "refined_netlist.xml");
#endif
}

void NLFiner::copy_template_cell_lib() {
  Library *design_prim_lib = _design->create_library(
      PRIMITIVE_LIB, false); // do not set external, else it won't output
  Library *cell_lib_prim_lib = _cell_lib->find_library(PRIMITIVE_LIB);
  ASSERT(cell_lib_prim_lib, "cannot find " + PRIMITIVE_LIB + " in cell lib");
  for (Module *mod : cell_lib_prim_lib->modules())
    mod->clone(mod->name(), design_prim_lib);
}

void NLFiner::remove_unused_module() {
  Library *design_work_lib = _design->find_library(BLOCK_LIB);
  ASSERT(design_work_lib, "cannot find " + BLOCK_LIB + " in design");
  design_work_lib->set_external(false);
  vector<string> FDP4_80_CELL_NAMES;
  FDP4_80_CELL_NAMES.push_back("slice");
  FDP4_80_CELL_NAMES.push_back("iob"); // TODO: add all cell names
  FDP4_80_CELL_NAMES.push_back("bufgmux");
  FDP4_80_CELL_NAMES.push_back("tbuf");
  FDP4_80_CELL_NAMES.push_back("ramb16");
  FDP4_80_CELL_NAMES.push_back("dll");
  FDP4_80_CELL_NAMES.push_back("bscan");
  FDP4_80_CELL_NAMES.push_back("mult18X18");
  FDP4_80_CELL_NAMES.push_back("vcc");
  vector<string> FDP3_300_CELL_NAMES;
  FDP3_300_CELL_NAMES.push_back("slice");
  FDP3_300_CELL_NAMES.push_back("iob");
  FDP3_300_CELL_NAMES.push_back("gclk");
  FDP3_300_CELL_NAMES.push_back("tbuf");
  FDP3_300_CELL_NAMES.push_back("blockram");
  FDP3_300_CELL_NAMES.push_back("gclkiob");
  FDP3_300_CELL_NAMES.push_back("bscan");
  FDP3_300_CELL_NAMES.push_back("dll");
  for (const string &cell : FDP4_80_CELL_NAMES) {
    if (Module *mod = design_work_lib->find_module(cell))
      design_work_lib->remove_module(mod);
  }
  for (const string &cell : FDP3_300_CELL_NAMES) {
    if (Module *mod = design_work_lib->find_module(cell))
      design_work_lib->remove_module(mod);
  }
}

void NLFiner::rebuild_verbose_instance(Instance *inst,
                                       map<string, size_t> &name_repo) {
  string inst_type(inst->module_type());
  Library *cell_work_lib = _cell_lib->find_library(BLOCK_LIB);
  ASSERT(cell_work_lib, "cannot find " + BLOCK_LIB + " in cell lib");
  Library *design_work_lib = _design->find_library(BLOCK_LIB);
  ASSERT(design_work_lib, "cannot find " + BLOCK_LIB + " in design");

  auto corresponding_mod =
      boost::find_if(cell_work_lib->modules(), [inst_type](Module *mod) {
        return mod->type() == inst_type;
      });
  ASSERT(corresponding_mod != cell_work_lib->modules().end(),
         "cannot find corresponding module \"" + inst_type + "\" in cell lib");

  string mod_name("c" + inst_type + "__" +
                  boost::lexical_cast<string>(name_repo[inst_type]++) + "__");
  Module *new_mod = (*corresponding_mod)->clone(mod_name, design_work_lib);
  FDU_LOG(DEBUG) << inst->name() << " change down_module to "
                 << new_mod->name();
  inst->set_down_module(new_mod);

  // redefine primitive module
  for (Instance *prim : new_mod->instances()) {
    Module *mod = _design->find_library(PRIMITIVE_LIB)
                      ->find_module(prim->down_module()->name());
    ASSERT(mod, "Cannot find corresponding downmodule in " + PRIMITIVE_LIB +
                    " in the design");
    FDU_LOG(DEBUG) << prim->name() << " change down_module to " << mod->name();
    prim->set_down_module(mod);
  }
}

void NLFiner::refine_instance(Instance *inst) {
  FDU_LOG(DEBUG) << "handle instance " << inst->name();

  //////////////////////////////////////////////////////////////////////////
  /// <1> load all configs
  vec_str configs;
  for (PropRepository::config_iter it = inst->configs().begin();
       it != inst->configs().end(); ++it) {
    string configValue = (*it)->name() + "::" + (*it)->string_value(inst);
    configs.push_back(configValue);
  }

  //////////////////////////////////////////////////////////////////////////
  /// <2> Construct the connection relationship of instances in slice or iob
  string block = to_lower_copy(inst->module_type());
  Module &inst_type = *(inst->down_module());
  for (const string &config : configs) {
    Config::bit_pair op = _repo->get_operation(block, config);
    char_separator<char> sep(":#\n\t");
    tokenizer<char_separator<char>> tok(config, sep);
    string prim = *(tok.begin()); // primitive name
    vec_str lib_names = _repo->find_lib_names(block, prim);
    restructure(inst_type, lib_names, op.first, op.second);
  }

  Property<bool> &NO_USER_LOGIC =
      create_temp_property<bool>(MODULE, "_NO_USER_LOGIC", false);
  if (block == "slice" && inst_type.property_value(NO_USER_LOGIC)) {
    vector<string> to_be_remove;
    for (Instance *inst : inst_type.instances()) {
      if (to_lower_copy(inst->down_module()->name()) == "vcc" ||
          to_lower_copy(inst->down_module()->name()) == "gnd")
        continue;

      to_be_remove.push_back(inst->down_module()->name());
    }
    op_remove(inst_type, to_be_remove);
  }

  // sweep the cell to remove all unused instances and nets
  // note that we may need several calls of opSweep()
  // since the sequence how each primitive will appear is indefinite
  size_t num_inst, num_net;
  do {
    num_inst = inst_type.num_instances();
    num_net = inst_type.num_nets();
    op_sweep(inst_type);
  } while (inst_type.num_instances() != num_inst ||
           inst_type.num_nets() != num_net);

  if (block == "slice" &&
      !inst_type.property_value(NO_USER_LOGIC)) // something related to the LUT
    handle_WSGEN(inst_type);
}

// restructure cell for a single op
void NLFiner::restructure(Module &cell, const vec_str &lib_names, string bit,
                          string op) {
  if (lib_names.size() == 0)
    return;

  // separating op
  boost::char_separator<char> sep(":");
  tokenizer<char_separator<char>> tok(op, sep);
  vector<string> operations(tok.begin(), tok.end()); // good
  // only for attributes, the op looks like attr:attr_name
  assert(operations.size() > 0);

  // analyze operation and assign to corresponding functions
  if (to_lower_copy(operations[0]) == "remove") {
    op_remove(cell, lib_names);
  } else if (to_lower_copy(operations[0]) == "mux") {
    op_mux(cell, lib_names, bit);
  } else if (to_lower_copy(operations[0]) == "vcc" ||
             to_lower_copy(operations[0]) == "gnd") {
    op_vccgnd(cell, lib_names, bit);
  } else if (to_lower_copy(operations[0]) == "attr") {
    assert(operations.size() == 2);
    op_attr(cell, lib_names, bit, operations[1]); // vOp[1] is property name
  } else if (to_lower_copy(operations[0]) == "dummy") {
    assert(operations.size() == 1);
    Property<bool> &no_user_logic =
        create_temp_property<bool>(MODULE, "_NO_USER_LOGIC");
    cell.set_property(no_user_logic, true);
  } else if (to_lower_copy(operations[0]) == "none") {
    return; // do nothing
  } else {
    cout << "[Error]: undefined operation type " << operations[0] << endl;
    exit(-1);
  }
}

// remove the primitives named lib_name(s) from cell
void NLFiner::op_remove(Module &cell, const vec_str &lib_names) {
  for (const string &prim : lib_names) {
    Module::inst_iter it = boost::find_if(cell.instances(), match_cell(prim));
    Instance *prim_pointer = (it == cell.instances().end()) ? 0 : *it;
    if (prim_pointer == nullptr) {
      ASSERT(0, "fail to find instance \"" + cell.name() + "::" + prim + "\"");
    }
    prim_pointer->release();
  }
}

// connect mux to the right input pin
// string bit is the pin to be connected
void NLFiner::op_mux(Module &cell, const vec_str &lib_names,
                     const string &bit) {
  for (const string &prim : lib_names) {
    Module::inst_iter it = boost::find_if(cell.instances(), match_cell(prim));
    Instance *prim_mux = (it == cell.instances().end()) ? 0 : *it;
    if (prim_mux == nullptr) {
      ASSERT(0, "fail to find instance \"" + cell.name() + "::" + prim + "\"");
    }

    string actual_bit(bit);
    // hard code, if the bit is 1, then the corresponding input is I1 ...
    if (actual_bit == "0")
      actual_bit = "I0";
    else if (actual_bit == "1")
      actual_bit = "I1";

    for (Pin *pin : prim_mux->pins()) {
      if (to_lower_copy(pin->port()->name()) != to_lower_copy(actual_bit) &&
          !pin->is_source()) { // the input of mux (not named bit) will be
                               // removed
        pin->disconnect();
      }
    }

    // sanity check
#ifdef _DEBUG
    for (Pin *pin : prim_mux->pins()) {
      if (to_lower_copy(pin->port()->name()) == to_lower_copy(actual_bit) ||
          pin->is_source()) {
        ASSERTS(pin->net() != nullptr);
      } else {
        ASSERTS(pin->net() == nullptr);
      }
    }
#endif
  }
}

// vcc/gnd operation for slice
void NLFiner::op_vccgnd(Module &cell, const vec_str &lib_names,
                        const string &bit) {
  ASSERTS(lib_names.size() == 1);

  string prim_name = lib_names[0]; // should be VCC or GND
  string port_name = bit;          // should be X or Y

  Module::inst_iter it =
      boost::find_if(cell.instances(), match_cell(prim_name));
  Instance *pmVccGnd = (it == cell.instances().end()) ? 0 : *it;
  ASSERTS(pmVccGnd != nullptr);

  Net *net;
  if (to_lower_copy(prim_name) == "vcc") {
    net = pmVccGnd->find_pin(VCC_OUT)->net();
  } else if (to_lower_copy(prim_name) == "gnd") {
    net = pmVccGnd->find_pin(GND_OUT)->net();
  } else {
    ASSERTS(0);
  }
  ASSERTS(net != nullptr);

  Pin *portPin = cell.find_port(port_name)->mpin();
  portPin->reconnect(net);
}

// add attributes to all cells in libName
void NLFiner::op_attr(Module &cell, const vec_str &lib_names, const string &bit,
                      const string &prop_name) {
  for (const string &prim : lib_names) {
    Module::inst_iter it = boost::find_if(cell.instances(), match_cell(prim));
    Instance *prim_attr =
        (it == cell.instances().end()) ? 0 : *it; // such as LUT, init0
    if (prim_attr == nullptr)
      continue;

    Property<string> &prop = create_temp_property<string>(INSTANCE, prop_name);
    prim_attr->set_property(prop, bit);
  }
}

void NLFiner::handle_WSGEN(Module &newCell) {
  Module::inst_iter itf = boost::find_if(newCell.instances(), match_cell("F"));
  Module::inst_iter itg = boost::find_if(newCell.instances(), match_cell("G"));
  Instance *f = (itf == newCell.instances().end()) ? 0 : *itf;
  Instance *g = (itg == newCell.instances().end()) ? 0 : *itg;
  string f_type = "";
  string g_type = "";
  Property<string> &type = create_temp_property<string>(INSTANCE, "type");
  if (f)
    f_type = f->property_value(type);
  if (g)
    g_type = g->property_value(type);

  if ((f_type == "LUT" || f_type == "") && (g_type == "LUT" || g_type == "")) {
    Module::inst_iter wit =
        boost::find_if(newCell.instances(), match_cell("WSGEN"));
    Instance *wsgen = (wit == newCell.instances().end()) ? 0 : *wit;
    if (wsgen)
      wsgen->release();
    if (f) {
      f->find_pin("WF1")->net()->release();
      f->find_pin("WF2")->net()->release();
      f->find_pin("WF3")->net()->release();
      f->find_pin("WF4")->net()->release();
      f->find_pin("WS")->net()->release();
    }
    if (g) {
      g->find_pin("WG1")->net()->release();
      g->find_pin("WG2")->net()->release();
      g->find_pin("WG3")->net()->release();
      g->find_pin("WG4")->net()->release();
      g->find_pin("WS")->net()->release();
    }
  }

  if (f_type == "LUT")
    handle_FG(f, "F_" + f_type);
  else if (f_type == "RAM")
    handle_FG(f, "F_" + f_type);
  else if (f_type == "ROM")
    handle_FG(f, "F_" + f_type);

  if (g_type == "LUT")
    handle_FG(g, "G_" + g_type);
  else if (g_type == "RAM")
    handle_FG(g, "G_" + g_type);
  else if (g_type == "ROM")
    handle_FG(g, "G_" + g_type);
}

void NLFiner::handle_FG(Instance *inst, const string &new_cell_name) {
  Library *cellLib = _design->find_library(PRIMITIVE_LIB);
  Module *cell = cellLib->find_module(new_cell_name);
  Instance *iinst = inst->module()->create_instance(inst->name(), cell);
  iinst->copy_property(inst);
  for (Pin *p : inst->pins()) {
    Pin *pinst = iinst->find_pin(p->name());
    if (p->net()) {
      pinst->connect(p->net());
    }
  }
  inst->release();
}

// sweep the whole cell to remove unused instances and nets
void NLFiner::op_sweep(Module &cell) {
  if (cell.type() == "VCC")
    return;

  //////////////////////////////////////////////////////////////////////////
  /// <1> delete instances that are unconnected
  Module::inst_iter inst_it = cell.instances().begin();
  while (inst_it != cell.instances().end()) {
    size_t num_conn_ipin = 0;
    size_t num_conn_outpin = 0;
    for (Pin *pin : inst_it->pins()) {
      if (pin->is_sink() && pin->net() != nullptr) { // input pin of the instance
        num_conn_ipin++;
      } else if (pin->is_source() &&
                 pin->net() != nullptr) { // output pin of the cell
        num_conn_outpin++;
      }
    }

    // do not remove vcc and gnd even it has no fanin
    if (to_lower_copy(inst_it->down_module()->name()) == "vcc" &&
        num_conn_outpin != 0) {
      ++inst_it;
      continue;
    }
    if (to_lower_copy(inst_it->down_module()->name()) == "gnd" &&
        num_conn_outpin != 0) {
      ++inst_it;
      continue;
    }

    // do not remove LUT if it is used as ram, even its output (port D) is empty
    string cell_type;
    Property<string> &type = create_temp_property<string>(INSTANCE, "type");
    Property<string> &TYPE = create_temp_property<string>(INSTANCE, "TYPE");
    if (inst_it->property_ptr(type) != nullptr)
      cell_type = *(inst_it->property_ptr(type));
    if (inst_it->property_ptr<string>(TYPE) != nullptr)
      cell_type = *(inst_it->property_ptr(TYPE));
    if (cell_type == "RAM") {
      ++inst_it;
      continue;
    }

    if (num_conn_ipin == 0 ||
        num_conn_outpin == 0) { // either no connected input pins or output pins
                                // means the instance/prmitive is unused
      for (Pin *pin : inst_it->pins())
        pin->disconnect();
      inst_it = cell.remove_instance(inst_it); // return the next valid iterator
    } else {
      ++inst_it;
    }
  }

  //////////////////////////////////////////////////////////////////////////
  /// <2> delete nets that are unconnected
  Module::net_iter net_it = cell.nets().begin();
  while (net_it != cell.nets().end()) { // note: pin's unhook is done in step 1
    if (net_it->sink_pins().begin() == net_it->sink_pins().end() ||
        net_it->source_pins().begin() == net_it->source_pins().end()) {
      while (!net_it->pins().empty()) { // all pins removed
        net_it->pins().back()->disconnect();
      }
      net_it = cell.remove_net(net_it); // moved to next net
    } else {
      ++net_it; // moved to next net
    }
  }

  //////////////////////////////////////////////////////////////////////////
  /// <3> remove unused cell ports
  Module::port_iter port_it = cell.ports().begin();
  while (port_it != cell.ports().end()) {
    if (!(port_it->mpin()->is_connected())) {
      port_it = cell.remove_port(port_it);
    } else {
      ++port_it;
    }
  }
}

} // namespace NLF
} // namespace FDU
