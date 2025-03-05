#include "PKApp.h"

namespace PACK {

using namespace std;
using namespace boost;

void Transformer::remove_dangling(bool is_step_one) {
  vector<Instance *> new_poss_dinsts;
  vector<Net *> new_poss_dnets;

  if (is_step_one) {
    // step 1: to remove dangling nets & instances produced when mapping
    // to hardware library
    while (poss_dangling_insts_.size() != 0 ||
           poss_dangling_nets_.size() != 0) {
      for (Net *net : poss_dangling_nets_) {
        bool is_dangling = is_net_dangling(net, new_poss_dinsts);
        if (is_dangling) {
          poss_dangling_insts_.insert(new_poss_dinsts.begin(),
                                      new_poss_dinsts.end());
          net->release();
        }
        new_poss_dinsts.clear();
      }
      poss_dangling_nets_.clear();
      for (Instance *inst : poss_dangling_insts_) {
        bool is_dangling = is_inst_dangling(inst, new_poss_dnets);
        if (is_dangling) {
          poss_dangling_nets_.insert(new_poss_dnets.begin(),
                                     new_poss_dnets.end());
          // emit signal here!
          top_cell_->before_inst_remove(static_cast<PKInstance *>(inst));
          inst->release();
        }
        new_poss_dnets.clear();
      }
      poss_dangling_insts_.clear();
    }
  } else {
    // step 2: to remove all instances packed into cluster & dangling nets
    if (definit_dangling_insts_.size() != 0) {
      for (Instance *inst : definit_dangling_insts_) {
        // emit signal here! this signal can be ignored, because here the cell
        // graph has stopped updating.
        top_cell_->before_inst_remove(static_cast<PKInstance *>(inst));
        inst->release();
      }

      for (Net *net : top_cell_->nets())
        if (net->source_pins().size() == 0 || net->sink_pins().size() == 0)
          new_poss_dnets.push_back(net);

      for (Net *net : new_poss_dnets)
        net->release();
    }
  }
}

void Transformer::remove_dangling() {
  vector<Instance *> new_poss_dinsts;
  vector<Net *> new_poss_dnets;

  // because we can't delete dangling element in place,
  // we should do the judgment twice
  for (Net *net : top_cell_->nets()) {
    if (is_net_dangling(net, new_poss_dinsts))
      poss_dangling_nets_.insert(net);
  }
  for (Instance *inst : top_cell_->instances())
    if (is_inst_dangling(inst, new_poss_dnets))
      poss_dangling_insts_.insert(inst);

  new_poss_dnets.clear();
  new_poss_dinsts.clear();
  while (poss_dangling_insts_.size() != 0 || poss_dangling_nets_.size() != 0) {
    for (Net *net : poss_dangling_nets_) {
      if (is_net_dangling(net, new_poss_dinsts)) {
        poss_dangling_insts_.insert(new_poss_dinsts.begin(),
                                    new_poss_dinsts.end());
        net->release();
      }
      new_poss_dinsts.clear();
    }
    poss_dangling_nets_.clear();
    for (Instance *inst : poss_dangling_insts_) {
      if (is_inst_dangling(inst, new_poss_dnets)) {
        poss_dangling_nets_.insert(new_poss_dnets.begin(),
                                   new_poss_dnets.end());
#ifdef _DEBUG
        cout << "Release Dangling Instance:\t" << inst->name() << endl;
#endif
        inst->release();
      }
      new_poss_dnets.clear();
    }
    poss_dangling_insts_.clear();
  }
}
/*
        bool Transformer::is_inst_dangling(Instance* inst, vector<Net*>&
   poss_dangling_nets)
        {
                bool has_input_net = false, has_output_net = false;
                bool has_input_pin = false, has_output_pin = false;

                for (Pin& pin: inst->pins()) {
                        if (pin.dir() == COS::INPUT)
                                has_input_pin = true;
                        else
                                has_output_pin = true; // assume it only has
   input/output pin

                        if (pin.net() != nullptr) {
                                poss_dangling_nets.push_back(pin.net());
                                if (pin.dir() == COS::OUTPUT)
                                        has_output_net = true;
                                else
                                        has_input_net = true;
                        }
                        if (has_input_net && has_output_net) break;
                }

                if ((!has_output_pin && has_input_net) ||
                        (!has_input_pin && has_output_net) ||
                        (has_input_net && has_output_net))
                        return false;

                return true;
        }
*/

bool Transformer::is_inst_dangling(Instance *inst,
                                   vector<Net *> &poss_dangling_nets) {
  bool has_input_net = false, has_output_net = false, has_inout_net = false;
  bool has_input_pin = false, has_output_pin = false, has_inout_pin = false;

  for (Pin *pin : inst->pins()) {
    if (pin->dir() == COS::INPUT)
      has_input_pin = true;
    else if (pin->dir() == COS::OUTPUT)
      has_output_pin = true;
    else if (pin->dir() == COS::INOUT)
      has_inout_pin = true;

    if (pin->net() != nullptr) {
      poss_dangling_nets.push_back(pin->net());
      if (pin->dir() == COS::INPUT)
        has_input_net = true;
      else if (pin->dir() == COS::OUTPUT)
        has_output_net = true;
      else if (pin->dir() == COS::INOUT)
        has_inout_net = true;
    }
    if (has_input_net && has_output_net)
      break;
  }

  if ((!has_output_pin && has_input_net) ||
      (!has_input_pin && has_output_net) || (has_input_net && has_output_net) ||
      (has_inout_pin && has_inout_net))
    return false;

  return true;
}

bool Transformer::is_net_dangling(Net *net,
                                  vector<Instance *> &poss_dangling_insts) {
  bool is_dangling =
      (net->source_pins().size() == 0 || net->sink_pins().size() == 0);
  if (is_dangling)
    for (Pin *pin : net->pins())
      if (!pin->is_mpin())
        poss_dangling_insts.push_back(pin->instance());
  return is_dangling;
}

void Transformer::execute(TranOpVec *op_vec) {
  for (TranOperation *op : *op_vec) {
    cur_op_ = op;
    cur_op_->load_target();
    for (const TranTestCase *test_case : op->test_cases()) {
      if (!test_case->test_expr()->execute())
        continue;
      for (const Instruct *instr : test_case->instructs())
        instr->execute();
    }
  }
}

void Transformer::reset_internal_struct() {
  created_cell_lktb_.clear();
  created_inst_lktb_.clear();
  created_net_lktb_.clear();
}

void Transformer::transform(Match *match) {
  cur_match_ = match;

  TranOpVec *op_vec = parse_rule(cur_match_->rule());

  cur_match_->load_matches();
  while (cur_match_->has_more_match()) {
    cur_match_->next_match();

    execute(op_vec);
    reset_internal_struct();
  }

  cur_match_ = nullptr;
}

void Transformer::transform(VCell *vcell) {
  cur_vcell_ = vcell;

  TranOpVec *op_vec = parse_rule(cur_vcell_->rule());

  execute(op_vec);
  reset_internal_struct();

  num_lut_in_cur_slice_ += cur_vcell_->rule()->rule_cell()->num_lut();
  num_ff_in_cur_slice_ += cur_vcell_->rule()->rule_cell()->num_ff();
  for (Match::InstPair &ipair : vcell->inst_pairs())
    definit_dangling_insts_.insert(ipair.image_inst);

  cur_vcell_ = nullptr;
}

bool Transformer::is_feasible(VCell *vcell) {
  static const int NUM_TEST = 3;
  static cstr_ptr pname_test[NUM_TEST] = {&PIN_NAME::CLK, &PIN_NAME::CE,
                                          &PIN_NAME::SR};
  static Property<string> *prop_test[NUM_TEST] = {
      &create_temp_property<string>(INSTANCE, PROP_NV::CKINV),
      &create_temp_property<string>(INSTANCE, PROP_NV::CEMUX),
      &create_temp_property<string>(INSTANCE, PROP_NV::SRMUX)};
  static Config *cfg_test[NUM_TEST] = {
      &create_config(CFG_NAME::SLICE, PROP_NV::CKINV),
      &create_config(CFG_NAME::SLICE, PROP_NV::CEMUX),
      &create_config(CFG_NAME::SLICE, PROP_NV::SRMUX)};

  ASSERTD(slice_cell_, "slice cell is nullptr when check feasible");
  ASSERTD(slice_inst_, "slice instance is nullptr when check feasible");

  // check for slice size constraint
  if (num_lut_in_cur_slice_ + vcell->rule()->rule_cell()->num_lut() >
      LUT_SIZE_PER_SLICE)
    return false;
  if (num_ff_in_cur_slice_ + vcell->rule()->rule_cell()->num_ff() >
      FF_SIZE_PER_SLICE)
    return false;

  // check for ff constraint
  PKInstance *ff_inst = nullptr;
  for (Match::InstPair &ipair : vcell->inst_pairs()) {
    if (ipair.rule_inst->down_module()->name() == CELL_NAME::FF) {
      ff_inst = ipair.image_inst;
      break;
    }
  }

  if (ff_inst != nullptr) {
    string slice_value = "";
    string ff_value = "";

    // 0 for clk check, 1 for ce check, 2 for sr check
    for (int i = 0; i < NUM_TEST; ++i) {
      Pin *slice_pin = slice_inst_->find_pin(*(pname_test[i]));
      VPort *vcell_port = vcell->find_port(*(pname_test[i]));
      if (vcell_port != nullptr && slice_pin != nullptr) {
        if (slice_pin->net() != nullptr &&
            vcell_port->vpin()->net() != slice_pin->net())
          return false;

        slice_value = slice_inst_->property_value(*(cfg_test[i]));
        ff_value = ff_inst->property_value(*(prop_test[i]));
        if (!slice_value.empty() && slice_value != ff_value)
          return false;
      }
    }
  }
  return true;
}

Transformer::TranOpVec *Transformer::parse_rule(Rule *rule) {
  RuleOpsMap::iterator fit = tran_operations_.find(rule);
  if (fit != tran_operations_.end())
    return fit->second;

  TranOpVec *op_vec = new TranOpVec();
  tran_operations_.insert(rule, op_vec);

  for (const Operation *op : rule->operations()) {
    TranOperation *tran_op = op_vec->add(new TranOperation(op->target(), this));
    for (const TestCase *tcase : op->test_cases()) {
      TranTestCase *tran_case = tran_op->create_test_cases();
      TestExprParser::parse_test_expr(tcase->test_expr, tran_case);
      for (const string &statement : tcase->instrcut_vec)
        InstructParser::parse_instruct(statement, tran_case);
    }
  }

  return op_vec;
}

string Transformer::make_name(const string &prefix, const string &suffix) {
  static string gbl_prefix;
  static std::map<string, int> name_idx_map;

  // initialize gbl_prefix to prevent name conflict,
  // suppose design and top_cell will not change
  if (gbl_prefix.empty()) {
    gbl_prefix = "_";

    for (Instance *inst : top_cell_->instances()) {
      while (find_first(inst->name(), gbl_prefix))
        gbl_prefix.append("_");
    }

    for (Net *net : top_cell_->nets()) {
      while (find_first(net->name(), gbl_prefix))
        gbl_prefix.append("_");
    }
  }

  // name making starts here
  string n(prefix);
  int idx;

  map<string, int>::iterator fit = name_idx_map.find(prefix);
  if (fit == name_idx_map.end()) {
    name_idx_map.insert(make_pair(prefix, 1));
    idx = 0;
  } else
    idx = (fit->second)++;

  return n.append(gbl_prefix)
      .append(lexical_cast<string>(idx))
      .append(gbl_prefix)
      .append(suffix);
}

Module *Transformer::find_cell(const NameInfo &cname_i) {
  if (cname_i.is_kw)
    return slice_cell_;

  NameCellMap::iterator fit = created_cell_lktb_.find(cname_i.name);
  return fit == created_cell_lktb_.end() ? nullptr : fit->second;
}

Instance *Transformer::find_instance(const NameInfo &iname_i) {
  // branch 0: name is KEY_WORD::SLICE_INST
  if (iname_i.is_kw)
    return slice_inst_;

  // branch 1: name is a fake name
  if (iname_i.is_fake) {
    NameInstMap::iterator fit = created_inst_lktb_.find(iname_i.name);
    return fit == created_inst_lktb_.end() ? nullptr : fit->second;
  }

  // branch 2: name is not fake and the target is SLICE_CELL
  if (cur_op_->target() != top_cell_) {
    /*sophie
    if (cur_op_->target()->name()!= KEY_WORD::SLICE_CELL)
    {
            NameCellMap::iterator fit =
    created_cell_lktb_.find(cur_op_->target()->name()); Module* cell = fit ==
    created_cell_lktb_.end() ? nullptr : fit->second; return
    cell->find_instance(iname_i.name);
    }
    else*/
    return cur_op_->target()->find_instance(iname_i.name);
  }

  // branch 3: name is not fake, the target is TopCell, like reconnect
  // and we are transform Match currently
  if (cur_match_ != nullptr) {
    RuleCell *rcell = cur_match_->rule()->rule_cell();
    RuleInstance *rinst = rcell->find_instance(iname_i.name);
    return rinst == nullptr ? nullptr : rinst->image();
  }

  // branch 4: name is not fake, the target is TopCell
  // and we are transform VCell currently
  if (cur_vcell_ != nullptr)
    return cur_vcell_->find_image_inst(iname_i.name);

  // should never happen
  ASSERT(false, "fatal error in Transformer::find_instance");
}
// TranOperation
void TranOperation::load_target() {
  if (target_ == KEY_WORD::TOP_CELL)
    target_cell_ = co_tf_->top_cell();
  else if (target_ == KEY_WORD::SLICE_CELL)
    target_cell_ = co_tf_->slice_cell();
}
} // namespace PACK