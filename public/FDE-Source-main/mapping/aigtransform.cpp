#include "log.h"
#include "mapping.hpp"
#include <boost/algorithm/string.hpp>
#include <stdint.h>

using namespace std;
using namespace COS;

namespace {
// pTruthtableItems need to be copied when flattern
Property<uint32_t> &pTruthtableItems =
    create_temp_property<uint32_t>(INSTANCE, "truthtable_items", 0, COPY);
Property<string> &pTruthtable = create_property<string>(MODULE, "truthtable");

struct IPin {
  Pin *pin;
  bool inv = false;
};
IPin operator!(IPin p) { return {p.pin, !p.inv}; }

class AigManager {
  Library *_workLib = nullptr;
  Module *_unitMod = nullptr;
  Module *_workMod = nullptr;
  int _index = 0;

public:
  AigManager(Design *pDesign);
  void makeAigInstance(Instance *inst);

private:
  Module *makeAigModule(Module *mod);
  Pin *makeAigNode(IPin A, IPin B, bool invY);
  Pin *And(IPin A, IPin B) { return makeAigNode(A, B, false); }
  Pin *Or(IPin A, IPin B) { return makeAigNode(!A, !B, true); }
  Pin *Identity(IPin A) { return makeAigNode(A, A, false); }
  using MkNode = Pin *(AigManager::*)(IPin A, IPin B); // And/Or
  IPin makeNodeTree(vector<IPin> pins, MkNode mkNode);
};

AigManager::AigManager(Design *pDesign) {
  FDU_LOG(INFO) << "createUnitAigModule";
  _workLib = pDesign->work_lib();
  _unitMod = _workLib->create_module("AigModuleAnd", "AIG"); // 构造AIG Moudule
  _unitMod->create_port("A", INPUT);
  _unitMod->create_port("B", INPUT);
  _unitMod->create_port("Y", OUTPUT);
}

void AigManager::makeAigInstance(Instance *inst) {
  string name = inst->name() + "_Aig";
  Instance *aigInst =
      inst->module()->create_instance(name, makeAigModule(inst->down_module()));
  for (Pin *pin : inst->pins()) {
    Pin *aigPin = aigInst->find_pin(pin->name() + "_Aig");
    aigPin->connect(pin->net());
  }
}

Pin *AigManager::makeAigNode(IPin A, IPin B, bool invY) {
  string index = to_string(_index++);
  string name = _workMod->name() + "_Node" + index;
  Instance *inst = _workMod->create_instance(name, _unitMod);
  auto connectNet = [&](Pin *p, string ipname) {
    Net *net = p->net();
    if (!net) {
      net = _workMod->create_net(p->name() + "_Net" + index);
      p->connect(net);
    }
    inst->find_pin(ipname)->connect(net);
  };
  connectNet(A.pin, "A");
  connectNet(B.pin, "B");

  uint32_t items = (1 << (!A.inv + !B.inv * 2)) ^ (invY * 15);
  inst->set_property(pTruthtableItems, items);

  return inst->find_pin("Y");
}

Module *AigManager::makeAigModule(Module *mod) {
  ASSERTSD(mod->type() == "COMB");
  string modName = mod->name() + "_Aig";
  _workMod = _workLib->find_module(modName);
  if (_workMod)
    return _workMod;
  _workMod = _workLib->create_module(modName, "AIG");

  vector<string> vCubes;
  boost::split(vCubes, mod->property_value(pTruthtable), boost::is_any_of("|"));
  ASSERT(vCubes.size() > 0, mod->name() + ": truthtable error");
  ASSERT(!vCubes[0].empty(), mod->name() + " is const!");

  vector<Port *> vInputPorts;
  Port *outputPort = nullptr;
  for (Port *port : mod->ports()) {
    Port *aigPort =
        _workMod->create_port(port->name() + "_Aig", port->dir(), port->type());
    if (aigPort->dir() == INPUT)
      vInputPorts.push_back(aigPort); // 这里没考虑INOUT型的port，以及总线
    if (aigPort->dir() == OUTPUT)
      outputPort = aigPort;
  }

  vector<IPin> vMidPins;
  for (string cube : vCubes) {
    ASSERT(cube.size() == vInputPorts.size(),
           mod->name() + ": truthtable error");
    vector<IPin> vInputPins;
    for (size_t i = 0; i < cube.size(); i++) {
      switch (cube[i]) {
      case '1':
        vInputPins.push_back({vInputPorts[i]->mpin(), false});
        break;
      case '0':
        vInputPins.push_back({vInputPorts[i]->mpin(), true});
        break;
      }
    }
    ASSERT(!vInputPins.empty(), mod->name() + ": truthtable error");
    vMidPins.push_back(makeNodeTree(vInputPins, &AigManager::And));
  }
  IPin oPin = makeNodeTree(vMidPins, &AigManager::Or);
  Pin *outputPin = !oPin.inv ? oPin.pin : Identity(oPin);

  Net *outputNet = _workMod->create_net(outputPin->name() + "_Net");
  outputPin->connect(outputNet);
  outputPort->mpin()->connect(outputNet);
  return _workMod;
}

IPin AigManager::makeNodeTree(vector<IPin> pins, MkNode mkNode) {
  while (pins.size() > 1) {
    pins.push_back({(this->*mkNode)(pins[0], pins[1])});
    pins.erase(pins.begin(), pins.begin() + 2);
  }
  return pins[0];
}
} // namespace

void MappingManager::doAigTransform() {
  auto am = AigManager{_pDesign};
  Module *mod = _pDesign->top_module();
  //	mod->flatten();

  vector<Instance *> combInstance;
  for (Instance *inst : mod->instances())
    if (inst->module_type() == "COMB") {
      combInstance.push_back(inst);
      am.makeAigInstance(inst);
    }
  mod->flatten();

  for (Instance *inst : combInstance)
    inst->release();
}
