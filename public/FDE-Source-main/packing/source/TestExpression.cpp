#include <boost/lexical_cast.hpp>

#include "PKMsg.h"
#include "Transformer.h"

namespace PACK {

using namespace std;
using namespace boost;
using namespace COS;

const int TTrue::NUM_PARAM = 0;
const int TFaninType::NUM_PARAM = 2;
const int TSliceFull::NUM_PARAM = 0;
const int TPinUsed::NUM_PARAM = 1;
const int TInstUsed::NUM_PARAM = 1;
const int TCurrentProcess::NUM_PARAM = 2;

// TestExprParser
TestExprParser::PTEHandler TestExprParser::parsing_array[] = {
    TTrue::parse,    TFaninType::parse, TSliceFull::parse,
    TPinUsed::parse, TInstUsed::parse,  TCurrentProcess::parse};

const char *const TTrue::TEXPR_NAME = "true";
const char *const TFaninType::TEXPR_NAME = "if_pin_fanin";
const char *const TSliceFull::TEXPR_NAME = "if_slice_full";
const char *const TPinUsed::TEXPR_NAME = "is_pin_used";
const char *const TInstUsed::TEXPR_NAME = "is_inst_used";
const char *const TCurrentProcess::TEXPR_NAME = "current_process";

static const char *const texpr_types[] = {
    TTrue::TEXPR_NAME,    TFaninType::TEXPR_NAME, TSliceFull::TEXPR_NAME,
    TPinUsed::TEXPR_NAME, TInstUsed::TEXPR_NAME,  TCurrentProcess::TEXPR_NAME};

static EnumStringMap<TExprType> itype_map(texpr_types);
istream &operator>>(istream &s, TExprType &etype) {
  try {
    etype = itype_map.readEnum(s);
    return s;
  } catch (bad_cast &e) {
    etype = UKWN_EXPR;
    return s;
  }
}

// TestExprParser
void TestExprParser::parse_test_expr(const string &statement,
                                     TranTestCase *test_case) {
  StrVec svec;
  bool is_invert = false;

  split_statement(statement, svec);
  ASSERT(svec.size() == 1 || svec.size() == 2,
         POErrMsg(POErrMsg::IVLD_STATE) % statement);

  if (svec[0][0] == '!') {
    is_invert = true;
    trim_left_if(svec[0], is_any_of("!"));
  }

  TExprType etype =
      lexical_cast<TExprType>(svec[0]); // convert a string to a enum type
  ASSERT(etype != UKWN_EXPR, POErrMsg(POErrMsg::UKWN_TEXPR) % svec[0]);

  string sparam;
  if (svec.size() > 1)
    sparam = svec[1];

  parsing_array[etype](sparam, test_case, is_invert);
}

// TestExpr Parsing
void TTrue::parse(const string &sparam, TranTestCase *test_case,
                  bool is_invert) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(sparam.empty(), POErrMsg(POErrMsg::UEXP_ARG_NUM) % TEXPR_NAME %
                             NUM_PARAM % pvec.size());

  test_case->create_test_expr<TTrue>(is_invert);
}

void TFaninType::parse(const string &sparam, TranTestCase *test_case,
                       bool is_invert) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() >= NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM_MORE) %
                                       TEXPR_NAME % NUM_PARAM % pvec.size());

  TFaninType *texpr = test_case->create_test_expr<TFaninType>(is_invert);

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % TEXPR_NAME);

  InstructParser::parse_name_info(texpr->iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  texpr->pname_ = avec[1]; // pin's name
  for (int i = 1; i < pvec.size(); ++i)
    texpr->tnames_.push_back(pvec[i]);
}

void TSliceFull::parse(const string &sparam, TranTestCase *test_case,
                       bool is_invert) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(sparam.empty(), POErrMsg(POErrMsg::UEXP_ARG_NUM) % TEXPR_NAME %
                             NUM_PARAM % pvec.size());

  test_case->create_test_expr<TSliceFull>(is_invert);
}

void TPinUsed::parse(const string &sparam, TranTestCase *test_case,
                     bool is_invert) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       TEXPR_NAME % NUM_PARAM % pvec.size());

  TPinUsed *texpr = test_case->create_test_expr<TPinUsed>(is_invert);

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % TEXPR_NAME);

  InstructParser::parse_name_info(texpr->iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  texpr->pname_ = avec[1];
}
/////////////////////////sophie///////////////
void TInstUsed::parse(const string &sparam, TranTestCase *test_case,
                      bool is_invert) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       TEXPR_NAME % NUM_PARAM % pvec.size());

  TInstUsed *texpr = test_case->create_test_expr<TInstUsed>(is_invert);

  StrVec avec;
  split_specific(pvec[0], avec, "@");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % TEXPR_NAME);

  InstructParser::parse_name_info(texpr->cname_i_, avec[0],
                                  KEY_WORD::SLICE_CELL);
  InstructParser::parse_name_info(texpr->iname_i_, avec[1],
                                  KEY_WORD::SLICE_INST);
}

void TCurrentProcess::parse(const string &sparam, TranTestCase *test_case,
                            bool is_invert) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       TEXPR_NAME % NUM_PARAM % pvec.size());

  TCurrentProcess *texpr =
      test_case->create_test_expr<TCurrentProcess>(is_invert);

  StrVec avec;
  split_specific(pvec[1], avec, "==");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[1] % TEXPR_NAME);

  InstructParser::parse_name_info(texpr->iname_i_, pvec[0],
                                  KEY_WORD::SLICE_INST);
  texpr->prop_name_ = avec[0];
  texpr->prop_value_ = avec[1];
}

/////////////////////////sophie///////////////
// TestExpr Execution
bool TTrue::execute() const { return is_inverted_ ^ true; }

bool TFaninType::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  Instance *inst = tf->find_instance(iname_i_);
  ASSERT(inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" % iname_i_.name %
                   TEXPR_NAME);

  Pin *pin = inst->find_pin(pname_);
  ASSERT(pin, TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % pname_ % TEXPR_NAME);

  if (pin->net() == nullptr)
    return is_inverted_ ^ false;

  ASSERT(pin->net()->source_pins().size() == 1,
         TFErrMsg(TFErrMsg::USAT_PIN_COND) % pname_ % "single-driven" %
             TEXPR_NAME);

  Pin *fanout_pin = *pin->net()->source_pins().begin();

  if (fanout_pin->is_mpin())
    return is_inverted_ ^ false;

  bool type_match = false;
  for (const string &tname : tnames_)
    if (tname == fanout_pin->instance()->down_module()->name()) {
      type_match = true;
      break;
    }

  return is_inverted_ ^ type_match;
}

bool TSliceFull::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  return is_inverted_ ^ tf->is_slice_full();
}

bool TPinUsed::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  Instance *inst = tf->find_instance(iname_i_);
  ASSERT(inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" % iname_i_.name %
                   TEXPR_NAME);

  PKPin *pin = static_cast<PKPin *>(inst->find_pin(pname_));
  ASSERT(pin, TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % pname_ % TEXPR_NAME);

  return is_inverted_ ^ pin->is_used();
}
/////////////////////////sophie///////////////
bool TInstUsed::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  Module *cell = tf->find_cell(cname_i_);
  ASSERT(cell,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "cell" % cname_i_.name % TEXPR_NAME);

  PKInstance *inst =
      static_cast<PKInstance *>(cell->find_instance(iname_i_.name));
  ASSERT(inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" % iname_i_.name %
                   TEXPR_NAME);

  return is_inverted_ ^ inst->is_used();
}

/////////////////////////sophie///////////////

extern Property<deque<string>> INDEX; // defined in instruction.cpp

bool TCurrentProcess::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  Instance *inst = tf->find_instance(iname_i_);
  ASSERT(inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" % iname_i_.name %
                   TEXPR_NAME);

  if (!inst->property_exist(INDEX))
    return false;

  bool flag = (inst->property_value<deque<string>>(INDEX)[0] == prop_value_);

  return is_inverted_ ^ flag;
}
} // namespace PACK