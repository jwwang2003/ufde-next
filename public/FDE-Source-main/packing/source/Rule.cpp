#include <boost/lexical_cast.hpp>
#include <fstream>

#include "PKUtils.h"
#include "Rule.h"
#include "io/xml/loadtxml.hpp"
#include "zfstream.h"
#include <rapidxml/rapidxml_utils.hpp> // file<>

namespace PACK {

using namespace std;
using namespace boost;
using namespace FDU::XML;
using COS::XML::NetlistHandler;
typedef rapidxml::file<> xml_file;

void Rule::set_rule_cell(RuleCell *cell) {
  ASSERT(!rule_cell_, "only one rule cell for a rule");
  //		rule_cell_ =
  // static_cast<RuleCell*>(rule_cell_lib.create_module(name,
  // KEY_WORD::RULE_CELL));
  rule_cell_ = cell;
  rule_cell_->co_rule_ = this;
}

RuleLibaray *PackRuleLib::create_rule_lib(const string &name) {

  switch (
      ++num_libs_) { // int
                     // num_libs中的1,2,3分别对应fdp_dc_plib.xml中的device_lib,macro_lib和normal_lib
  case 1:
    hw_rule_lib_.reset(new RuleLibaray(name));
    return hw_rule_lib_.get();
  case 2:
    macro_rule_lib_.reset(new RuleLibaray(name));
    return macro_rule_lib_.get();
  case 3:
    pack_rule_lib_.reset(new RuleLibaray(name));
    return pack_rule_lib_.get();
  default:
    ASSERT(0, "only three rule library allow");
  }
}

//  load Rule
class RuleCellLoader : public NetlistHandler {
  Library *lib_;

public:
  RuleCellLoader(Library *lib) : lib_(lib) {}
  RuleCell *load_cell(xml_node *node);

protected:
  Port *load_port(xml_node *node, Module *mod);
  Instance *load_instance(xml_node *node, Module *mod);
};

RuleCell *RuleCellLoader::load_cell(xml_node *node) {
  set_attribute(
      node, "type",
      KEY_WORD::
          RULE_CELL); // KEY_WORD在PKConfig.h中定义，set_attribute最底层调用rapidxml中的函数，rule的cell中本来无type这一属性，为了与fdp_cell_lib.xml中对应，好调用统一的操作，所以这里加上这一属性
  RuleCell *cell = dynamic_cast<RuleCell *>(
      load_module(node, lib_)); // 调用基类NetlistHandler中的load_module函数
  ASSERT(cell, "parse failed. cell: create cell error");
  cell->cnt_resource();
  return cell;
}

Port *RuleCellLoader::load_port(xml_node *node, Module *mod) {
  RulePort *rport =
      dynamic_cast<RulePort *>(NetlistHandler::load_port(node, mod));
  ASSERT(rport, "parse failed. port: create port error");
  if (get_attribute(node, "connected") == "true")
    rport->set_connect();
  if (get_attribute(node, "bus_ignore") == "true")
    rport->set_bus_ignore();
  return rport;
}

Instance *RuleCellLoader::load_instance(xml_node *node, Module *mod) {
  RuleInstance *inst =
      dynamic_cast<RuleInstance *>(NetlistHandler::load_instance(node, mod));
  if (inst && get_attribute(node, "lock") == "false")
    inst->clear_rule_lock();
  return inst;
}

class RuleLibLoader {
  PackRuleLib *pack_rule_lib_;
  RuleCellLoader cell_loader_;

public:
  RuleLibLoader(PackRuleLib *pl, Library *rule_lib)
      : pack_rule_lib_(pl), cell_loader_(rule_lib) {}
  void parse(istream &s);

private:
  void load_libs(xml_node *root);
  void load_rule(xml_node *node, RuleLibaray::Layer *layer, int layer_id);
  void load_oper(xml_node *node, Rule *rule);
  void load_test(xml_node *node, Operation *oper);
};

void RuleLibLoader::parse(istream &s) {
  xml_file f(s); // f指向fdp_dc_plib.xml
  xml_document doc;
  doc.parse<0>(f.data()); // rapidxml中提供的操作，解析fdp_dc_plib.xml网表
  xml_node *root = doc.first_node();
  load_libs(root);
}

void RuleLibLoader::load_libs(xml_node *root) {
  foreach_child(
      lib_node, root,
      "library") { // fdp_dc_plib的root结点为libraries,下一级子结点为library
    RuleLibaray *lib = pack_rule_lib_->create_rule_lib(get_attribute(
        lib_node,
        "name")); // 这个pack_rule_lib_为RuleLibLoader的成员，而非PackRuleLib中的pack_rule_lib_
    int layer_id =
        0; // get_attribute函数就是获得xml文件中结点的属性，libraries等仅有name属性
    foreach_child(layer_node, lib_node, "layer") {
      RuleLibaray::Layer *layer =
          lib->create_layer(); // layer在程序中以lay_id标识，所以不用get_attribute
      foreach_child(rule_node, layer_node, "rule")
          load_rule(rule_node, layer, layer_id);
      ++layer_id;
    }
  }
}

void RuleLibLoader::load_rule(xml_node *node, RuleLibaray::Layer *layer,
                              int layer_id) {
  xml_node *cell_node = node->first_node("left")->first_node("cell");
  RuleCell *cell = cell_loader_.load_cell(
      cell_node); // cell_loader_为RuleCellLoader类的对象，最终调用NetlistHandler类的load_module然后，一层，一层调用下去
  if (cell->num_nets() == 0)
    return; // incomplete cell, ignore this rule
  Rule *rule = layer->add(new Rule(
      layer_id,
      get_attribute(
          node,
          "name"))); // node一直都没变，指向rule结点,在这里，如果前面的条件均满足，则可以创建一个rule，放入RuleLibrary中
  rule->set_rule_cell(cell);
  xml_node *right_node = node->first_node("right");
  foreach_child(oper_node, right_node, "operations") load_oper(oper_node, rule);
}

void RuleLibLoader::load_oper(xml_node *node, Rule *rule) {
  string target = get_attribute(node, "target");
  if (target.empty())
    target = KEY_WORD::TOP_CELL;
  Operation *oper = rule->create_operation(target);
  foreach_child(test_node, node, "test") load_test(test_node, oper);
}

void RuleLibLoader::load_test(xml_node *node, Operation *oper) {
  TestCase *test_case = oper->create_case();
  test_case->test_expr = get_attribute(node, "cond");
  foreach_child(op_node, node, "op")
      test_case->instrcut_vec.push_back(op_node->value());
}

void PackRuleLib::load(const string &fn, Design *design) {
  CosFactory::pointer old_factory = CosFactory::set_factory(new RuleFactory());

  RuleLibLoader loader(this, design->find_or_create_library("RULE_CELL_LIB"));
  ifstrm ifs(fn.c_str()); // fn为fdp_dc_plib.xml
  loader.parse(ifs);      // 解析fdp_dc_plib.xml文件
  CosFactory::set_factory(old_factory);
}

} // namespace PACK