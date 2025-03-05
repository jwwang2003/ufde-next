#include "plc_const_infer.h"
#include "arch/archlib.hpp"
#include "plc_factory.h"
#include "plc_utils.h"

#include <tuple> // tie

namespace FDU {
namespace Place {

using namespace std;
using namespace ARCH;

void ConstantsGener::generate_constants(TDesign *design) {
  Net *gnd_net, *vcc_net;
  tie(gnd_net, vcc_net) = find_constants_nets(design);

  if (gnd_net || vcc_net) {
    generate_dummy_cell(design, gnd_net, vcc_net);
    generate_dummy_inst(design, gnd_net, vcc_net);
  }
}

pair<Net *, Net *> ConstantsGener::find_constants_nets(TDesign *design) {
  int gnd_net_count = 0, vcc_net_count = 0;
  Net *gnd_net = nullptr, *vcc_net = nullptr;
  Property<string> &types = create_temp_property<string>(COS::NET, NET::TYPE);
  for (Net *net : design->top_module()->nets()) {

    string type = net->property_value(types);
    if (type == NET::GND) {
      gnd_net_count++;
      gnd_net = net;
    } else if (type == NET::VCC) {
      vcc_net_count++;
      vcc_net = net;
    } else {
    }
  }
  ASSERT(gnd_net_count <= 1 && vcc_net_count <= 1,
         (CONSOLE::PLC_ERROR % "more than 1 vcc net or 1 gnd net."));

  return make_pair(gnd_net, vcc_net);
}

void ConstantsGener::generate_dummy_cell(TDesign *design, Net *gnd_net,
                                         Net *vcc_net) {
  Library *work_lib = design->find_or_create_library("work_lib");
  _const_slice_cell =
      work_lib->create_module("DUMMY_CONSTANTS_GENER", CELL::SLICE);
  if (gnd_net)
    generate_gnd_part(design);
  if (vcc_net)
    generate_vcc_part(design);
}

void ConstantsGener::generate_dummy_inst(TDesign *design, Net *gnd_net,
                                         Net *vcc_net) {
  _const_slice_inst = design->top_module()->create_instance(
      "inst_dummy_constants_gener", _const_slice_cell);
  if (gnd_net) {
    Config &G_LUT = create_config("instance", "G#LUT");
    Config &GYMUX = create_config("instance", "GYMUX");
    Config &YUSED = create_config("instance", "YUSED");
    _const_slice_inst->pins().find("Y")->connect(gnd_net);
    _const_slice_inst->set_property(G_LUT, "D=0");
    _const_slice_inst->set_property(GYMUX, "G");
    _const_slice_inst->set_property(GYMUX, "0");
  }
  if (vcc_net) {
    Config &F_LUT = create_config("instance", "F#LUT");
    Config &FXMUX = create_config("instance", "FXMUX");
    Config &XUSED = create_config("instance", "XUSED");
    _const_slice_inst->pins().find("X")->connect(vcc_net);
    _const_slice_inst->set_property<string>(F_LUT, "D=1");
    _const_slice_inst->set_property<string>(FXMUX, "F");
    _const_slice_inst->set_property<string>(XUSED, "0");
  }
}

// use G-LUT to produce constant 0 (output Y)
void ConstantsGener::generate_gnd_part(TDesign *design) {
  // ports
#ifdef fdp3000k

  _const_slice_cell->create_port("G1", INPUT);
  _const_slice_cell->create_port("G2", INPUT);
  _const_slice_cell->create_port("G3", INPUT);
  _const_slice_cell->create_port("G4", INPUT);
  _const_slice_cell->create_port("Y", OUTPUT);
#else
  _const_slice_cell->create_port("Y", OUTPUT);
#endif
  // instances
  Library *tcell_lib = design->find_or_create_library("template_cell_lib");
#ifdef fdp3000k
  Instance *lut4 = _const_slice_cell->create_instance(
      "dummy_gnd_G", tcell_lib->modules().find("LUT4"));
#else
  Instance *lut4 = _const_slice_cell->create_instance(
      "dummy_gnd_G", tcell_lib->modules().find("G"));
#endif

  Instance *gymux = _const_slice_cell->create_instance(
      "gymux", tcell_lib->modules().find("GYMUX"));
  Instance *yused = _const_slice_cell->create_instance(
      "yused", tcell_lib->modules().find("YUSED"));

  // nets
  Net *net = nullptr;
#ifdef fdp3000k
  net = _const_slice_cell->create_net("G1");
  _const_slice_cell->ports().find("G1")->mpin()->connect(net);
  lut4->pins().find("I1")->connect(net);

  net = _const_slice_cell->create_net("G2");
  _const_slice_cell->ports().find("G2")->mpin()->connect(net);
  lut4->pins().find("I2")->connect(net);

  net = _const_slice_cell->create_net("G3");
  _const_slice_cell->ports().find("G3")->mpin()->connect(net);
  lut4->pins().find("I3")->connect(net);

  net = _const_slice_cell->create_net("G4");
  _const_slice_cell->ports().find("G4")->mpin()->connect(net);
  lut4->pins().find("I4")->connect(net);

  net = _const_slice_cell->create_net("g_d");
  lut4->pins().find("O")->connect(net);
  gymux->pins().find("G")->connect(net);

  net = _const_slice_cell->create_net("gymux_o");
  gymux->pins().find("O")->connect(net);
  yused->pins().find("I")->connect(net);

  net = _const_slice_cell->create_net("Y");
  yused->pins().find("O")->connect(net);
  _const_slice_cell->ports().find("Y")->mpin()->connect(net);

#else
  net = _const_slice_cell->create_net("g_d");
  lut4->pins().find("D")->connect(net);
  gymux->pins().find("G")->connect(net);

  net = _const_slice_cell->create_net("gymux_o");
  gymux->pins().find("O")->connect(net);
  yused->pins().find("I")->connect(net);

  net = _const_slice_cell->create_net("Y");
  yused->pins().find("O")->connect(net);
  _const_slice_cell->ports().find("Y")->mpin()->connect(net);
#endif
}

// use F-LUT to produce constant 1 (output X)
void ConstantsGener::generate_vcc_part(TDesign *design) {
  // ports
#ifdef fdp3000k
  _const_slice_cell->create_port("F1", INPUT);
  _const_slice_cell->create_port("F2", INPUT);
  _const_slice_cell->create_port("F3", INPUT);
  _const_slice_cell->create_port("F4", INPUT);
  _const_slice_cell->create_port("X", OUTPUT);
#else
  _const_slice_cell->create_port("X", OUTPUT);
#endif

  // instances
  Library *tcell_lib = design->find_or_create_library("template_cell_lib");
  Instance *fxmux = _const_slice_cell->create_instance(
      "fxmux", tcell_lib->modules().find("FXMUX"));
  Instance *xused = _const_slice_cell->create_instance(
      "xused", tcell_lib->modules().find("XUSED"));
#ifdef fdp3000k
  Instance *lut4 = _const_slice_cell->create_instance(
      "dummy_vcc_F", tcell_lib->modules().find("LUT4"));
#else
  Instance *lut4 = _const_slice_cell->create_instance(
      "dummy_vcc_F", tcell_lib->modules().find("F"));
#endif

  // nets
  Net *net = nullptr;
#ifdef fdp3000k
  net = _const_slice_cell->create_net("F1");
  _const_slice_cell->ports().find("F1")->mpin()->connect(net);
  lut4->pins().find("I1")->connect(net);

  net = _const_slice_cell->create_net("F2");
  _const_slice_cell->ports().find("F2")->mpin()->connect(net);
  lut4->pins().find("I2")->connect(net);

  net = _const_slice_cell->create_net("F3");
  _const_slice_cell->ports().find("F3")->mpin()->connect(net);
  lut4->pins().find("I3")->connect(net);

  net = _const_slice_cell->create_net("F4");
  _const_slice_cell->ports().find("F4")->mpin()->connect(net);
  lut4->pins().find("I4")->connect(net);

  net = _const_slice_cell->create_net("f_d");
  lut4->pins().find("O")->connect(net);
  fxmux->pins().find("F")->connect(net);

  net = _const_slice_cell->create_net("fxmux_o");
  fxmux->pins().find("O")->connect(net);
  xused->pins().find("I")->connect(net);

  net = _const_slice_cell->create_net("X");
  xused->pins().find("O")->connect(net);
  _const_slice_cell->ports().find("X")->mpin()->connect(net);
#else
  net = _const_slice_cell->create_net("f_d");
  lut4->pins().find("D")->connect(net);
  fxmux->pins().find("F")->connect(net);

  net = _const_slice_cell->create_net("fxmux_o");
  fxmux->pins().find("O")->connect(net);
  xused->pins().find("I")->connect(net);

  net = _const_slice_cell->create_net("X");
  xused->pins().find("O")->connect(net);
  _const_slice_cell->ports().find("X")->mpin()->connect(net);
#endif
}

} // namespace Place
} // namespace FDU