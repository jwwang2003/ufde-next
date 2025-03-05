/*! \file Rule.h
 *  \brief  Header of rule description
 *  \author S.C. Leung
 *  \date   2009.05 - 2009.06
 *
 */

#ifndef _RULE_H
#define _RULE_H

#include <boost/scoped_ptr.hpp>
#include <utility>

#include "PKFactory.h"

namespace PACK {

struct TestCase // 语句类
{
  typedef string TestExpr;
  typedef std::vector<string> InstructSet; // 存放从fdp_dc_plib.xml中装载的指令

  TestExpr
      test_expr; // 测试类语句，如slice_full,true,对应fdp_dc_plib.xml中的test结点
  InstructSet instrcut_vec; // 存放从fdp_dc_plib.xml中装载的指令
};

class Operation { // 操作类
public:
  Operation(const string &target) : target_(target) {}

  const string &target() const { return target_; }

  //////////////////////////////////////////////////////////////////////////
  //  TestCases
  typedef PtrVector<TestCase>::const_range_type const_cases_type;
  typedef PtrVector<TestCase>::const_iterator const_case_iter;

  size_t num_cases() const { return test_cases_.size(); }
  const_cases_type test_cases() const { return test_cases_.range(); }
  TestCase *create_case() { return test_cases_.add(new TestCase); }

private:
  string target_;
  PtrVector<TestCase> test_cases_;
};

class Rule { // 规则类
public:
  Rule(int layer_idx, const string &n = "")
      : layer_idx_(layer_idx), name_(n), rule_cell_(0) {}

  int layer_idx() const { return layer_idx_; }
  const string &name() const { return name_; }
  bool is_rule_set() const { return rule_cell_; }
  const RuleCell *rule_cell() const { return rule_cell_; }
  RuleCell *rule_cell() { return rule_cell_; }
  void set_rule_cell(RuleCell *cell);

  bool operator==(const Rule &rhs) const { return this == &rhs; }
  bool operator!=(const Rule &rhs) const { return this != &rhs; }

  //////////////////////////////////////////////////////////////////////////
  //  Operations
  typedef PtrVector<Operation>::const_range_type const_operations_type;
  typedef PtrVector<Operation>::const_iterator const_operation_iter;

  size_t num_operations() const { return operations_.size(); }
  const_operations_type operations() const { return operations_.range(); }
  Operation *create_operation(const string &target = KEY_WORD::TOP_CELL) {
    return operations_.add(new Operation(target));
  }

private:
  int layer_idx_;
  string name_;
  RuleCell *rule_cell_;
  PtrVector<Operation> operations_;
};

class RuleLibaray {
public:
  RuleLibaray(const string &name = "") : name_(name) {}

  typedef PtrVector<Rule> Layer;
  typedef PtrVector<Layer>::const_range_type const_layers_type;
  typedef PtrVector<Layer>::range_type layers_type;
  typedef PtrVector<Layer>::const_iterator const_layer_iter;
  typedef PtrVector<Layer>::iterator layer_iter;

  size_t num_layers() const { return layers_.size(); }
  const_layers_type layers() const { return layers_.range(); }
  layers_type layers() { return layers_.range(); }
  Layer *create_layer() { return layers_.add(new Layer()); }
  //		void  remove_layer(Layer* layer)     {
  // layers_.erase(boost::range::find(layers_, *layer)); }

private:
  string name_;
  PtrVector<Layer> layers_;
};

class PackRuleLib {
public:
  PackRuleLib() : num_libs_(0) {}

  RuleLibaray *hw_rule_lib() const { return hw_rule_lib_.get(); }
  RuleLibaray *pack_rule_lib() const { return pack_rule_lib_.get(); }
  RuleLibaray *macro_rule_lib() const { return macro_rule_lib_.get(); }

  RuleLibaray *create_rule_lib(const string &);
  void load(const string &, Design *);

private:
  boost::scoped_ptr<RuleLibaray> hw_rule_lib_;
  boost::scoped_ptr<RuleLibaray> pack_rule_lib_;
  /*sophie begin:sep 15th*/
  boost::scoped_ptr<RuleLibaray> macro_rule_lib_;
  /*sophie end:for adding another macro lib*/
  int num_libs_;
};

// iostream
// std::istream& operator >> (std::istream& s, bool& b);
// std::ostream& operator << (std::ostream& s, bool b);

} // namespace PACK

#endif