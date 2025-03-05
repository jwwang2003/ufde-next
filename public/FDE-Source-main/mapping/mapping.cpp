#include "mapping.hpp"
#include "io/usingedif.h"
#include "io/usingverilog.h"
#include "log.h"
#include "report/report.h"

#include <algorithm> // max_element, min_element
#include <memory>    // unique_ptr
#include <sstream>
#include <stdint.h>
#include <tuple> // tie

using namespace std;
using namespace COS;

namespace {
// ------------------------------------
// MapCut

uint32_t pinSignature(Pin *pin) {
  static Property<uint32_t> pSignature;
  static uint32_t signature = 0;
  if (pin->property_exist(pSignature))
    return pin->property_value(pSignature);
  signature <<= 1;
  if (!signature)
    signature = 1;
  pin->set_property(pSignature, signature);
  return signature;
}

struct MapCut {
  Instance *root = nullptr;
  vector<COS::Pin *> leaves;
  uint32_t signature;
  uint32_t truthtable = 0;
  int level = 0;
  int area = 0;
  bool valid = false;

  MapCut(Pin *leaf)
      : leaves{leaf}, signature{pinSignature(leaf)}, truthtable{2} {}
  MapCut(MapCut *cut0, MapCut *cut1, Instance *cutRoot, size_t lutSize);

  void addLeaf(Pin *pin) {
    for (Pin *p : leaves)
      if (pin == p)
        return;
    leaves.push_back(pin);
  }
  int numLeaves() const { return leaves.size(); }
};

Property<MapCut *> pBestCut{nullptr};
Property<uint32_t> &pTruthtableItems =
    create_temp_property<uint32_t>(INSTANCE, "truthtable_items", 0, COPY);

uint32_t calcTruthtable(int n, vector<int> index0, vector<int> index1,
                        uint32_t tt0, uint32_t tt1, uint32_t ttRoot) {
  vector<int> state(n);
  uint32_t items = 0;
  for (int st = 0; st < (1 << n); st++) {
    for (int index = 0; index < n; index++)
      state[index] = !!(st & (1 << index));
    uint32_t input0 = 0, input1 = 0;
    for (size_t i = 0; i < index0.size(); i++) {
      input0 |= state[index0[i]] << i;
    }
    for (size_t i = 0; i < index1.size(); i++) {
      input1 |= state[index1[i]] << i;
    }
    int result0 = !!(tt0 & (1 << input0));
    int result1 = !!(tt1 & (1 << input1));

    if (ttRoot & (1 << (result0 + result1 * 2)))
      items |= (1 << st);
  }
  return items;
}

size_t oneCount(uint32_t n) {
  size_t count = 0;
  for (; n != 0; n &= n - 1)
    ++count;
  return count;
}

MapCut::MapCut(MapCut *cut0, MapCut *cut1, Instance *cutRoot, size_t lutSize)
    : root{cutRoot}, signature{cut0->signature | cut1->signature} {
  if (oneCount(signature) > lutSize)
    return; // invalid

  for (Pin *leaf : cut0->leaves)
    addLeaf(leaf);
  for (Pin *leaf : cut1->leaves)
    addLeaf(leaf);
  if (leaves.size() > lutSize)
    return; // invalid

  vector<int> index0(cut0->leaves.size(), -1);
  vector<int> index1(cut1->leaves.size(), -1);
  for (size_t i = 0; i < leaves.size(); i++) {
    for (size_t j = 0; j < cut0->leaves.size(); j++)
      if (cut0->leaves[j] == leaves[i])
        index0[j] = i;
    for (size_t j = 0; j < cut1->leaves.size(); j++)
      if (cut1->leaves[j] == leaves[i])
        index1[j] = i;
  }
  truthtable = calcTruthtable(leaves.size(), std::move(index0), std::move(index1),
                              cut0->truthtable, cut1->truthtable,
                              root->property_value(pTruthtableItems));

  int maxLeafLevel = 0, totalLeafArea = 0;
  for (Pin *leaf : leaves) {
    if (leaf->instance() && leaf->instance()->module_type() == "AIG") {
      MapCut *bestCut = leaf->instance()->property_value(pBestCut);
      ASSERTS(bestCut && bestCut->level > 0 && bestCut->area > 0);
      maxLeafLevel = max(maxLeafLevel, bestCut->level);
      totalLeafArea += bestCut->area;
    }
  }
  level = maxLeafLevel + 1;
  area = totalLeafArea + 1;
  valid = true;
}

// ------------------------------------
// getNodeDfs
bool isDfsRoot(Instance *node) {
  if (node->module_type() != "AIG")
    return false;
  Pin *opin = node->find_pin("Y");
  for (Pin *pin : opin->net()->sink_pins())
    if (!pin->is_mpin() && pin->instance()->module_type() == "AIG")
      return false;
  return true;
}

Pin *getFaninPin(Instance *inst, const string &pinName) {
  auto src_pins = inst->find_pin(pinName)->net()->source_pins();
  return (src_pins.size() > 0) ? src_pins[0] : nullptr;
}

Instance *getFaninInstance(Instance *inst, const string &pinName) {
  auto pin = getFaninPin(inst, pinName);
  return pin ? pin->instance() : nullptr;
}

Property<bool> pVisited;
void getNodeDfsRec(vector<Instance *> &nodes, Instance *node) {
  if (!node || node->module_type() != "AIG" || node->property_value(pVisited))
    return;
  node->set_property(pVisited, true);

  getNodeDfsRec(nodes, getFaninInstance(node, "A"));
  getNodeDfsRec(nodes, getFaninInstance(node, "B"));

  nodes.push_back(node);
}

vector<Instance *> getNodeDfs(Module *top) {
  auto nodes = vector<Instance *>{};
  pVisited.clear();
  for (Instance *inst : top->instances())
    if (isDfsRoot(inst))
      getNodeDfsRec(nodes, inst);
  pVisited.clear();
  return nodes;
}

// ------------------------------------
// Cut Enumeration

using CutPtr = unique_ptr<MapCut>;
using CutVec = PtrVector<MapCut>;
Property<CutVec> pAllCuts;

bool betterCut(const MapCut *a, const MapCut *b) {
  return a && b && tie(a->level, a->area) < tie(b->level, b->area);
}

void addCut(CutVec &allCuts, CutPtr cut) {
  const int maxCuts = 63;
  if (!cut->valid)
    return;
  allCuts.push_back(cut.release()); // transfer ownership
  if (allCuts.size() <= maxCuts)
    return;
  auto worst =
      max_element(allCuts.rbegin(), allCuts.rend(), betterCut).base() - 1;
  allCuts.erase(worst);
}

auto getAllCuts(Instance *node, string pinName) {
  auto leafCut = std::make_unique<MapCut>(getFaninPin(node, pinName));
  auto allCuts = vector<MapCut *>{leafCut.get()};
  if (auto inst = getFaninInstance(node, pinName); inst) {
    auto &pAll = inst->property_cref(pAllCuts);
    allCuts.insert(allCuts.end(), pAll.begin(), pAll.end());
  }
  return make_pair(std::move(leafCut), allCuts);
}

void enumNode(Instance *node, int lutSize) {
  auto [leafCutA, allCutsA] = getAllCuts(node, "A");
  auto [leafCutB, allCutsB] = getAllCuts(node, "B");
  auto &allCuts = node->property_ref(pAllCuts);
  for (auto cutA : allCutsA)
    for (auto cutB : allCutsB)
      addCut(allCuts, make_unique<MapCut>(cutA, cutB, node, lutSize));
  auto bestCut = *min_element(allCuts.begin(), allCuts.end(), betterCut);
  node->set_property(pBestCut, bestCut);
}

// ------------------------------------
// LUT Transform
bool isLutRoot(Instance *node) {
  if (!node || node->module_type() != "AIG")
    return false;
  Pin *opin = node->find_pin("Y");
  for (Pin *pin : opin->net()->sink_pins())
    if (pin->is_mpin() || pin->instance()->module_type() != "AIG")
      return true;
  return false;
}

vector<Instance *> getLutRoots(vector<Instance *> &dfsNodes) {
  pVisited.clear();
  vector<Instance *> lutRootNodes;
  int max_level = 0;
  for (Instance *node : dfsNodes)
    if (isLutRoot(node)) {
      lutRootNodes.push_back(node);
      node->set_property(pVisited, true);
      max_level = max(max_level, node->property_value(pBestCut)->level);
    }
  FDU_LOG(INFO) << "max level: " << max_level;
  for (size_t i = 0; i < lutRootNodes.size(); i++) {
    Instance *node = lutRootNodes[i];
    for (Pin *leaf : node->property_value(pBestCut)->leaves) {
      Instance *nodeA = leaf->instance();
      if (nodeA && (nodeA->module_type() == "AIG") &&
          !nodeA->property_value(pVisited)) {
        lutRootNodes.push_back(nodeA);
        nodeA->set_property(pVisited, true);
      }
    }
  }
  pVisited.clear();
  return lutRootNodes;
}

void createLutModule(Design *pDesign, int maxSize) {
  COS::Library *lib = pDesign->find_library("cell_lib");
  for (int k = 2; k <= maxSize; k++) {
    string lutModName = "LUT" + to_string(k);
    COS::Module *mod = lib->create_module(lutModName, "LUT");
    for (int i = 0; i < k; i++) {
      string portName = "ADR" + to_string(i);
      mod->create_port(portName, COS::INPUT);
    }
    mod->create_port("O", COS::OUTPUT);
  }
}

void setTruthtableProperty(Instance *inst, uint32_t truthtable) {
  static auto &pTruthtable =
      COS::create_property<string>(COS::INSTANCE, "INIT");
  static auto &pTruthtable1 =
      COS::create_property<string>(COS::INSTANCE, "INIT_1");
  static auto &pTruthtable2 =
      COS::create_property<string>(COS::INSTANCE, "INIT_2");

  int k = inst->num_pins() - 1;
  ASSERTS(k >= 2 && k <= 5);
  stringstream ss;
  ss << hex << uppercase << truthtable;
  string ts = ss.str();
  unsigned long len = (1 << (k - 2));
  while (ts.size() < len)
    ts = "0" + ts;
  if (ts.size() == 8) {
    inst->set_property(pTruthtable1, ts.substr(0, 4));
    inst->set_property(pTruthtable2, ts.substr(4, 4));
  } else {
    inst->set_property(pTruthtable, ts);
  }
}

void makeLut(Instance *node, int index) {
  MapCut *cut = node->property_value(pBestCut);
  ASSERTS(cut);
  int k = cut->leaves.size();
  if (k == 1) {
    k = 2;
    cut->leaves.push_back(cut->leaves[0]);
    FDU_LOG(INFO) << "INV to LUT2";
  }
  Module *mod = node->module();
  Library *lib = mod->library()->design()->find_library("cell_lib");
  Module *mLut = lib->find_module("LUT" + to_string(k));
  Instance *inst = mod->create_instance("L" + to_string(index), mLut);
  Net *onet = cut->root->find_pin("Y")->net();
  onet->rename("net_" + inst->name());
  inst->find_pin("O")->connect(onet);

  for (int i = 0; i < k; i++) {
    Pin *ipin = inst->find_pin("ADR" + to_string(i));
    ipin->connect(cut->leaves[i]->net());
  }
  setTruthtableProperty(inst, cut->truthtable);
}

// ------------------------------------
// Report
map<string, int> countCells(const Module *pMod) {
  map<string, int> result;
  for (Instance *pObj : pMod->instances()) {
    auto type = pObj->down_module()->type();
    auto down_name = pObj->down_module()->name();
    if (type == "LUT") {
      ++result[type];
      ++result[down_name];
    } else if (type == "FFLATCH" || type == "MACRO") {
      ++result[type];
      ++result[type + "_" + down_name];
    }
  }
  return result;
}
} // namespace

// ------------------------------------
// MappingManager
void MappingManager::doReadDesign() {
  _pDesign->load("xml", _args.cellLib);
  if (_args.flow != Args::Yosys)
    return _pDesign->load(_args.inputType, _args.inputFile);

  ASSERTS(_args.inputType == "edif"); // edif from Yosys
  IO::using_edif();
  // cell_lib�ĸ�����Ҫ��loadedif�д���������ֱ�Ӵ��ݲ�����ʹ��property���ݸ���Ҫ��
  auto &pRename = create_temp_property<string>(DESIGN, "cell_lib_rename");
  _pDesign->set_property(pRename, "cell_lib=LIB");
  _pDesign->load("edif", _args.inputFile);
  _pDesign->top_module()->library()->rename("work_lib");
  // convert truthtable
  auto &pLut_init = create_property<int64_t>(INSTANCE, "LUT_INIT");
  for (Instance *inst : _pDesign->top_module()->instances())
    if (inst->property_exist(pLut_init))
      setTruthtableProperty(inst, inst->property_value(pLut_init));
  pLut_init.clear();
}

void MappingManager::doWriteDesign() {
  _pDesign->save("xml", _args.outputFile, _args.fEncry);
  IO::using_verilog();
  if (!_args.verilogFile.empty())
    _pDesign->save("verilog", _args.verilogFile);
}

void MappingManager::doMapCut() {
  auto dfsNodes = getNodeDfs(_pDesign->top_module());

  for (Instance *node : dfsNodes)
    enumNode(node, _args.lutSize);

  createLutModule(_pDesign, _args.lutSize);
  int index = 0;
  for (Instance *node : getLutRoots(dfsNodes))
    makeLut(node, index++);
  for (Instance *node : dfsNodes)
    node->release();

  vector<Net *> netsToRemove;
  for (Net *net : _pDesign->top_module()->nets())
    if (net->num_pins() == 0)
      netsToRemove.push_back(net);
  for (Net *net : netsToRemove)
    net->release();
}

void MappingManager::doReport() {
  using namespace FDU::RPT;
  string report_file_name = _args.prjName + ".rpt";
  Report rpt;
  rpt.set_app("map");
  rpt.set_label("FDE Mapping Report File");
  rpt.set_design(_pDesign->name());

  Section *sec_type = rpt.create_section("Type_Count", "Type Count");
  Table *table_type = sec_type->create_table("General_Table");
  table_type->create_column("Type_Name", "Type Name");
  table_type->create_column("Count", "Count");

  auto count_info = countCells(_pDesign->top_module());
  for (auto [name, count] : count_info) {
    Row *row = table_type->create_row();
    row->set_item("Type_Name", name);
    row->set_item("Count", count);
  }

  rpt.write(report_file_name);
}
