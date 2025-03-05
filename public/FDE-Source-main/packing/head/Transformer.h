#ifndef _TRANSFORMER_H
#define _TRANSFORMER_H

#include "Instruction.h"
#include "Match.h"
#include "TestExpression.h"

#include <boost/ptr_container/ptr_map.hpp>
#include <boost/scoped_ptr.hpp>

using namespace COS;

namespace PACK {

class TranOperation;
class Transformer;

class TranTestCase {
public:
  TranTestCase(TranOperation *to) : co_operation_(to) {}

  // instructions
  typedef PtrVector<Instruct>::const_range_type const_instructs_type;

  size_t num_instructs() const { return instructs_.size(); }
  const_instructs_type instructs() const { return instructs_.range(); }

  template <typename I> inline I *create_instruct();
  template <typename T> inline T *create_test_expr(bool);

  TestExpr *test_expr() const { return test_expr_.get(); }
  TranOperation *co_operation() const { return co_operation_; }

private:
  TranOperation *co_operation_;
  boost::scoped_ptr<TestExpr> test_expr_;
  PtrVector<Instruct> instructs_;
};

template <typename I> inline I *TranTestCase::create_instruct() {
  I *pi = new I(this);
  instructs_.add(pi);
  return pi;
}

template <typename T> inline T *TranTestCase::create_test_expr(bool is_invert) {
  T *pt = new T(this, is_invert);
  test_expr_.reset(pt);
  return pt;
}

class TranOperation {
public:
  TranOperation(const string &target, Transformer *tf)
      : co_tf_(tf), target_(target) {}

  void load_target();
  Transformer *transformer() const { return co_tf_; }
  PKCell *target() const { return target_cell_; }

  // test cases
  typedef PtrVector<TranTestCase>::const_range_type const_cases_type;
  typedef PtrVector<TranTestCase>::const_iterator const_case_iter;

  size_t num_cases() const { return test_cases_.size(); }
  const_cases_type test_cases() const { return test_cases_.range(); }
  TranTestCase *create_test_cases() {
    return test_cases_.add(new TranTestCase(this));
  }

private:
  Transformer *co_tf_;
  string target_;
  PKCell *top_cell_;
  PKCell *target_cell_;
  PtrVector<TranTestCase> test_cases_;
};

class Transformer {
public:
  // constructor
  Transformer()
      : cur_match_(nullptr), cur_vcell_(nullptr), cur_op_(nullptr), top_cell_(nullptr),
        slice_cell_(nullptr), slice_inst_(nullptr), slice_full_flag_(false),
        num_lut_in_cur_slice_(0), num_ff_in_cur_slice_(0) {}

  void transform(Match *);
  void remove_dangling(bool);
  void remove_dangling();

  void transform(VCell *);
  bool is_feasible(VCell *);

  string make_name(const string &, const string &);
  Instance *find_instance(const NameInfo &);
  Module *find_cell(const NameInfo &);

  // for safety, maybe you should concatenate the up-cell name to the instance
  // name
  PKCell *top_cell() const { return top_cell_; }
  Match *cur_match() const { return cur_match_; } // add by sophie
  PKCell *slice_cell() const { return slice_cell_; }
  bool is_slice_full() const { return slice_full_flag_; }
  PKInstance *slice_inst() const { return slice_inst_; }

  void set_top_cell(PKCell *tc) { top_cell_ = tc; }
  void set_sfull_flag() { slice_full_flag_ = true; }
  void clear_sfull_flag() { slice_full_flag_ = false; }

  void set_slice_cell(PKCell *sc) { slice_cell_ = sc; }
  void set_slice_inst(PKInstance *si) { slice_inst_ = si; }
  void reset_lut_num() { num_lut_in_cur_slice_ = 0; }
  void reset_ff_num() { num_ff_in_cur_slice_ = 0; }

  void insert_new_inst(const string &iname, Instance *inst) {
    created_inst_lktb_.insert(std::make_pair(iname, inst));
  }
  void insert_new_net(const string &nname, Net *net) {
    created_net_lktb_.insert(std::make_pair(nname, net));
  }
  void insert_new_cell(const string &cname, Module *cell) {
    created_cell_lktb_.insert(std::make_pair(cname, cell));
  }
  void insert_poss_dangling_inst(Instance *inst) {
    poss_dangling_insts_.insert(inst);
  }
  void insert_poss_dangling_net(Net *net) { poss_dangling_nets_.insert(net); }

private:
  typedef std::map<string, Instance *> NameInstMap;
  typedef std::map<string, Net *> NameNetMap;
  typedef std::map<string, Module *> NameCellMap;
  typedef PtrVector<TranOperation> TranOpVec;
  typedef boost::ptr_map<Rule *, TranOpVec> RuleOpsMap;

private:
  void execute(TranOpVec *);
  void reset_internal_struct();
  TranOpVec *parse_rule(Rule *);
  bool is_inst_dangling(Instance *, vector<Net *> &);
  bool is_net_dangling(Net *net, vector<Instance *> &);

private:
  unique_list<Instance *> poss_dangling_insts_;
  unique_list<Net *> poss_dangling_nets_;
  unique_list<Instance *> definit_dangling_insts_;

  Match *cur_match_;
  VCell *cur_vcell_;
  TranOperation *cur_op_;
  RuleOpsMap tran_operations_;

  PKCell *top_cell_;
  PKCell *slice_cell_;
  PKInstance *slice_inst_;
  bool slice_full_flag_;
  int num_lut_in_cur_slice_;
  int num_ff_in_cur_slice_;

  // special type for name making
  NameInstMap created_inst_lktb_;
  NameNetMap created_net_lktb_;
  NameCellMap created_cell_lktb_;
};

} // namespace PACK
#endif
