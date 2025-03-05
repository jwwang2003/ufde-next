#include <boost/lexical_cast.hpp>
#include <boost/phoenix/core.hpp>

#include "PKApp.h"
#include "PKMsg.h"

namespace PACK {

using namespace std;
using namespace boost;
using namespace COS;

Property<string> &INIT =
    create_property<string>(INSTANCE, "INIT", std::string(), COS::COPY);
Property<string> &INIT_HEX =
    create_property<string>(INSTANCE, "INIT_HEX", std::string(), COS::COPY);

const int ICreateInst::NUM_PARAM = 3;
const int ICreateSlice::NUM_PARAM = 3;
const int ICreateIob::NUM_PARAM = 5; // sophie
const int ICreateNet::NUM_PARAM = 1;
const int ICloneCell::NUM_PARAM = 4;
const int IReconnect::NUM_PARAM = 2;
const int IConnect::NUM_PARAM = 2;   // czh
const int IExconnect::NUM_PARAM = 2; // czh
const int IReconnectOpt::NUM_PARAM = 3;
const int IUnhook::NUM_PARAM = 1;
const int IDupConnect::NUM_PARAM = 2;
const int IUsePin::NUM_PARAM = 1;
const int ISetProperty::NUM_PARAM = 2;
const int ICloneProperty::NUM_PARAM = 4; // sophie
const int ICopyProperty::NUM_PARAM = 4;
const int ISetIndex::NUM_PARAM = 2;
const int IShareNet::NUM_PARAM = 2;
const int IReconnect2::NUM_PARAM = 3;

//  Instruction Parser
smatch InstructParser::regex_m;

InstructParser::PIHandler InstructParser::parsing_array[] = {
    ICreateInst::parse,
    ICreateSlice::parse // czh
    ,
    ICreateIob::parse,
    ICreateNet::parse,
    ICloneCell::parse,
    IReconnect::parse,
    IConnect::parse // add by czh
    ,
    IExconnect::parse // add by czh
    ,
    IReconnectOpt::parse,
    IUnhook::parse,
    IDupConnect::parse,
    IUsePin::parse,
    ISetProperty::parse,
    ICloneProperty::parse,
    ICopyProperty::parse,
    ISetIndex::parse,
    IShareNet::parse,
    IReconnect2::parse};

const char *const ICreateInst::INSTR_NAME = "create_inst";
const char *const ICreateSlice::INSTR_NAME = "create_slice"; // czh
const char *const ICreateIob::INSTR_NAME = "create_iob";
const char *const ICreateNet::INSTR_NAME = "create_net";
const char *const ICloneCell::INSTR_NAME = "clone_cell";
const char *const IReconnect::INSTR_NAME = "reconnect";
const char *const IConnect::INSTR_NAME = "connect";     // add by czh
const char *const IExconnect::INSTR_NAME = "exconnect"; // add by czh
const char *const IReconnectOpt::INSTR_NAME = "reconnect_opt";
const char *const IUnhook::INSTR_NAME = "unhook";
const char *const IDupConnect::INSTR_NAME = "dupconnect";
const char *const IUsePin::INSTR_NAME = "use";
const char *const ISetProperty::INSTR_NAME = "set_property";
const char *const ICloneProperty::INSTR_NAME = "clone_property";
const char *const ICopyProperty::INSTR_NAME = "copy_property";
const char *const ISetIndex::INSTR_NAME = "set_index";
const char *const IShareNet::INSTR_NAME = "share_net";
const char *const IReconnect2::INSTR_NAME = "reconnect2";

static const char *const instr_types[] = {ICreateInst::INSTR_NAME,
                                          ICreateSlice::INSTR_NAME // czh
                                          ,
                                          ICreateIob::INSTR_NAME,
                                          ICreateNet::INSTR_NAME,
                                          ICloneCell::INSTR_NAME,
                                          IReconnect::INSTR_NAME,
                                          IConnect::INSTR_NAME // czh
                                          ,
                                          IExconnect::INSTR_NAME // czh
                                          ,
                                          IReconnectOpt::INSTR_NAME,
                                          IUnhook::INSTR_NAME,
                                          IDupConnect::INSTR_NAME,
                                          IUsePin::INSTR_NAME,
                                          ISetProperty::INSTR_NAME,
                                          ICloneProperty::INSTR_NAME,
                                          ICopyProperty::INSTR_NAME,
                                          ISetIndex::INSTR_NAME,
                                          IShareNet::INSTR_NAME,
                                          IReconnect2::INSTR_NAME};

static EnumStringMap<InstrucType> itype_map(instr_types);
istream &operator>>(istream &s, InstrucType &itype) {
  try {
    itype = itype_map.readEnum(s);
    return s;
  } catch (bad_cast &e) {
    itype = UKWN_INSTR;
    return s;
  }
}

// Note: this is function's param must be the old instance(Lut[1-4]) created by
// Synplify NOT LUT created by our substitution!! Default each lut's input pin
// is connected!
int num_oldinst_inpins(Instance *inst) {
  int num = 0;
  for (Pin *pin : inst->pins()) {
    if (pin->is_sink())
      num++;
  }
  return num;
}

void InstructParser::parse_instruct(const string &statement,
                                    TranTestCase *test_case) {
  StrVec svec;

  split_statement(statement, svec);
  ASSERT(svec.size() == 2, POErrMsg(POErrMsg::IVLD_STATE) % statement);

  InstrucType itype = lexical_cast<InstrucType>(svec[0]);
  ASSERT(itype != UKWN_INSTR, POErrMsg(POErrMsg::UKWN_INSTR) % svec[0]);

  parsing_array[itype](svec[1], test_case);
}

void ICreateInst::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());
  // regex the fake name
  ASSERT(match_fake_name(pvec[0], InstructParser::regex_m),
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  ICreateInst *new_instr = test_case->create_instruct<ICreateInst>();

  new_instr->iname_i_.name = pvec[0];
  new_instr->iname_i_.is_fake = true;
  new_instr->name_prefix_ = InstructParser::regex_m[1];
  new_instr->name_suffix_ = InstructParser::regex_m[2];

  InstructParser::parse_name_info(new_instr->cname_i_, pvec[1],
                                  KEY_WORD::SLICE_CELL);

  // fake name with"{}";except slice, iob, gclkiob and gclk,the model cell is in
  // the hw_lib
  if (!new_instr->cname_i_.is_kw && !new_instr->cname_i_.is_fake) {
    Library *lib = PKApp::instance().design().libs().find(pvec[2]);
    ASSERT(lib,
           POErrMsg(POErrMsg::UFND_ELEM) % "library" % pvec[2] % INSTR_NAME);

    new_instr->model_ = lib->find_module(pvec[1]);
    ASSERT(new_instr->model_,
           POErrMsg(POErrMsg::UFND_ELEM) % "cell" % pvec[1] % INSTR_NAME);
  }
}
/*sophie begin: AUG 3TH*/
void ICreateIob::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  ASSERT(match_fake_name(pvec[0], InstructParser::regex_m),
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  ICreateIob *new_instr = test_case->create_instruct<ICreateIob>();

  new_instr->iname_i_.name = pvec[0];
  new_instr->iname_i_.is_fake = true;
  new_instr->name_prefix_ = InstructParser::regex_m[1];
  new_instr->name_suffix_ = InstructParser::regex_m[2];
  new_instr->pname_i_.name = pvec[3];
  new_instr->pin_name_ = pvec[4];

  InstructParser::parse_name_info(new_instr->cname_i_, pvec[1],
                                  KEY_WORD::SLICE_CELL);

  // fake name with"{}";except slice, iob, gclkiob and gclk,the model cell is in
  // the hw_lib
  if (!new_instr->cname_i_.is_kw && !new_instr->cname_i_.is_fake) {
    Library *lib = PKApp::instance().design().libs().find(pvec[2]);
    ASSERT(lib,
           POErrMsg(POErrMsg::UFND_ELEM) % "library" % pvec[2] % INSTR_NAME);

    new_instr->model_ = lib->find_module(pvec[1]);
    ASSERT(new_instr->model_,
           POErrMsg(POErrMsg::UFND_ELEM) % "cell" % pvec[1] % INSTR_NAME);
  }
} /*sophie end: for iob name clarification*/
void ICreateNet::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  ASSERT(match_fake_name(pvec[0], InstructParser::regex_m),
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  ICreateNet *new_instr = test_case->create_instruct<ICreateNet>();

  new_instr->nname_i_.name = pvec[0];
  new_instr->nname_i_.is_fake = true;
  new_instr->name_prefix_ = InstructParser::regex_m[1];
  new_instr->name_suffix_ = InstructParser::regex_m[2];
}

void ICloneCell::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  ASSERT(match_fake_name(pvec[0], InstructParser::regex_m),
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  ICloneCell *new_instr = test_case->create_instruct<ICloneCell>();

  new_instr->cname_i_.name = pvec[0];
  new_instr->cname_i_.is_fake = true;
  new_instr->name_prefix_ = InstructParser::regex_m[1];
  new_instr->name_suffix_ = InstructParser::regex_m[2];

  new_instr->cell_owner_ = PKApp::instance().design().libs().find(pvec[1]);
  ASSERT(new_instr->cell_owner_,
         POErrMsg(POErrMsg::UFND_ELEM) % "library" % pvec[1] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->model_cname_i_, pvec[2],
                                  KEY_WORD::SLICE_CELL);

  if (!new_instr->model_cname_i_.is_kw && !new_instr->model_cname_i_.is_fake) {
    Library *lib = PKApp::instance().design().libs().find(pvec[3]);
    ASSERT(lib,
           POErrMsg(POErrMsg::UFND_ELEM) % "library" % pvec[3] % INSTR_NAME);

    new_instr->model_ = lib->find_module(pvec[2]);
    ASSERT(new_instr->model_,
           POErrMsg(POErrMsg::UFND_ELEM) % "cell" % pvec[2] % INSTR_NAME);
  }
}

void IReconnect::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  IReconnect *new_instr = test_case->create_instruct<IReconnect>();

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->old_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->old_pname_ = avec[1];

  split_specific(pvec[1], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[1] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->new_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->new_pname_ = avec[1];
}

// add by czh: begin 2009-9-17
void ICreateSlice::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());
  // regex the fake name
  ASSERT(match_fake_name(pvec[0], InstructParser::regex_m),
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  ICreateSlice *new_instr = test_case->create_instruct<ICreateSlice>();

  new_instr->iname_i_.name = pvec[0];
  new_instr->iname_i_.is_fake = true;
  new_instr->name_prefix_ = InstructParser::regex_m[1];
  new_instr->name_suffix_ = InstructParser::regex_m[2];

  InstructParser::parse_name_info(new_instr->cname_i_, pvec[1],
                                  KEY_WORD::SLICE_CELL);

  // fake name with"{}";except slice, iob, gclkiob and gclk,the model cell is in
  // the hw_lib
  if (!new_instr->cname_i_.is_kw && !new_instr->cname_i_.is_fake) {
    Library *lib = PKApp::instance().design().libs().find(pvec[2]);
    ASSERT(lib,
           POErrMsg(POErrMsg::UFND_ELEM) % "library" % pvec[2] % INSTR_NAME);

    new_instr->model_ = lib->find_module(pvec[1]);
    ASSERT(new_instr->model_,
           POErrMsg(POErrMsg::UFND_ELEM) % "cell" % pvec[1] % INSTR_NAME);
  }
}
void IConnect::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  IConnect *new_instr = test_case->create_instruct<IConnect>();

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->old_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->old_pname_ = avec[1];

  split_specific(pvec[1], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[1] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->new_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->new_pname_ = avec[1];
}
void IExconnect::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  IExconnect *new_instr = test_case->create_instruct<IExconnect>();

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->old_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->old_pname_ = avec[1];

  split_specific(pvec[1], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[1] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->new_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->new_pname_ = avec[1];
}
// add by czh: end
void IReconnectOpt::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() >= NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM_MORE) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  IReconnectOpt *new_instr = test_case->create_instruct<IReconnectOpt>();

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->old_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->old_pname_ = avec[1];

  for (size_t i = 1; i < pvec.size(); ++i) {
    split_specific(pvec[i], avec, ".");
    ASSERT(avec.size() == 2,
           POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[i] % INSTR_NAME);

    new_instr->opt_inames_i_.push_back(NameInfo());
    InstructParser::parse_name_info(new_instr->opt_inames_i_.back(), avec[0],
                                    KEY_WORD::SLICE_INST);
    new_instr->opt_pnames_.push_back(avec[1]);
  }
}

void IUnhook::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  IUnhook *new_instr = test_case->create_instruct<IUnhook>();

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->pname_ = avec[1];
}

void IDupConnect::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  IDupConnect *new_instr = test_case->create_instruct<IDupConnect>();

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->src_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->src_pname_ = avec[1];

  split_specific(pvec[1], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[1] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->tar_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->tar_pname_ = avec[1];
}

void IUsePin::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() >= NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM_MORE) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  IUsePin *new_instr = test_case->create_instruct<IUsePin>();

  StrVec avec;
  for (size_t i = 0; i < pvec.size(); ++i) {
    split_specific(pvec[i], avec, ".");
    ASSERT(avec.size() == 2,
           POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[i] % INSTR_NAME);

    new_instr->opt_inames_i_.push_back(NameInfo());
    InstructParser::parse_name_info(new_instr->opt_inames_i_.back(), avec[0],
                                    KEY_WORD::SLICE_INST);
    new_instr->opt_pnames_.push_back(avec[1]);
  }
}

void ISetProperty::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;
  StrVec avec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() >= NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  ISetProperty *new_instr = test_case->create_instruct<ISetProperty>();

  InstructParser::parse_name_info(new_instr->iname_i_, pvec[0],
                                  KEY_WORD::SLICE_INST);
  for (size_t i = 1; i < pvec.size(); i++) {
    split_specific(pvec[i], avec, "::");
    ASSERT(avec.size() == 2, "ISetProperty::parse Error!");
    new_instr->prop_map_[avec[0]] = avec[1];
  }
}

void ICloneProperty::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() == NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  ICloneProperty *new_instr = test_case->create_instruct<ICloneProperty>();

  InstructParser::parse_name_info(new_instr->src_iname_i_, pvec[0],
                                  KEY_WORD::SLICE_INST);
  InstructParser::parse_name_info(new_instr->tar_iname_i_, pvec[1],
                                  KEY_WORD::SLICE_INST);
  new_instr->prop_name_ = pvec[2];
  new_instr->default_value_ = pvec[3];
}

void ICopyProperty::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() >= NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  ICopyProperty *new_instr = test_case->create_instruct<ICopyProperty>();
  InstructParser::parse_name_info(new_instr->src_iname_i_, pvec[0],
                                  KEY_WORD::SLICE_INST);
  InstructParser::parse_name_info(new_instr->tar_iname_i_, pvec[2],
                                  KEY_WORD::SLICE_INST);
  new_instr->src_prop_name_ = pvec[1];
  new_instr->tar_prop_name_ = pvec[3];
}

void ISetIndex::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;
  StrVec avec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() >= NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  ISetIndex *new_instr = test_case->create_instruct<ISetIndex>();

  InstructParser::parse_name_info(new_instr->iname_i_, pvec[0],
                                  KEY_WORD::SLICE_INST);
  for (size_t i = 1; i < pvec.size(); i++) {
    split_specific(pvec[i], avec, "::");
    ASSERT(avec.size() == 2, "ISetIndex::parse Error!");
    // if(!new_instr->prop_map_.count(avec[0])){
    vector<string> tvec = new_instr->prop_map_[avec[0]];
    tvec.push_back(avec[1]);
    new_instr->prop_map_[avec[0]] = tvec;
    //}else{
    //	vector<string> tvec(1,avec[1]);
    //	new_instr->prop_map_[avec[0]] = tvec;
    //}
  }
}

void IShareNet::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() >= NUM_PARAM && pvec.size() % 2 == 0,
         POErrMsg(POErrMsg::UEXP_ARG_NUM) % INSTR_NAME % NUM_PARAM %
             pvec.size());

  IShareNet *new_instr = test_case->create_instruct<IShareNet>();

  for (size_t i = 0; i < pvec.size() / 2; i++) {
    // create a share condition
    ShareCond sc = ShareCond();

    // parse share_nets
    StrVec sn;
    split_specific(pvec[i << 1], sn, "==");
    ASSERT(sn.size() == 2, "IShareNet::parse Error!");

    StrVec avec1, avec2;
    split_specific(sn[0], avec1, ".");
    InstructParser::parse_name_info(sc.sn_iname1_, avec1[0],
                                    KEY_WORD::SLICE_INST);
    sc.sn_pname1_ = avec1[1];
    split_specific(sn[1], avec2, ".");
    InstructParser::parse_name_info(sc.sn_iname2_, avec2[0],
                                    KEY_WORD::SLICE_INST);
    sc.sn_pname2_ = avec2[1];

    // parse todo
    StrVec todo;
    split_specific(pvec[(i << 1) + 1], todo, ":=");
    ASSERT(todo.size() == 2, "IShareNet::parse Error!");

    StrVec todo1, todo2;
    split_specific(todo[0], todo1, ".");
    InstructParser::parse_name_info(sc.do_iname1_, todo1[0],
                                    KEY_WORD::SLICE_INST);
    sc.do_pname1_ = todo1[1];
    split_specific(todo[1], todo2, ".");
    InstructParser::parse_name_info(sc.do_iname2_, todo2[0],
                                    KEY_WORD::SLICE_INST);
    sc.do_pname2_ = todo2[1];

    // add the share condition
    new_instr->share_conds_.push_back(sc);
  }
}
void IReconnect2::parse(const string &sparam, TranTestCase *test_case) {
  StrVec pvec;

  split_params(sparam, pvec);
  ASSERT(pvec.size() >= NUM_PARAM, POErrMsg(POErrMsg::UEXP_ARG_NUM_MORE) %
                                       INSTR_NAME % NUM_PARAM % pvec.size());

  IReconnect2 *new_instr = test_case->create_instruct<IReconnect2>();

  StrVec avec;
  split_specific(pvec[0], avec, ".");
  ASSERT(avec.size() == 2,
         POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[0] % INSTR_NAME);

  InstructParser::parse_name_info(new_instr->old_iname_i_, avec[0],
                                  KEY_WORD::SLICE_INST);
  new_instr->old_pname_ = avec[1];

  for (size_t i = 1; i < pvec.size(); ++i) {
    split_specific(pvec[i], avec, ".");
    ASSERT(avec.size() == 2,
           POErrMsg(POErrMsg::UEXP_ARG_FMT) % pvec[i] % INSTR_NAME);

    new_instr->opt_inames_i_.push_back(NameInfo());
    InstructParser::parse_name_info(new_instr->opt_inames_i_.back(), avec[0],
                                    KEY_WORD::SLICE_INST);
    new_instr->opt_pnames_.push_back(avec[1]);
  }
}

// Instruction execution
Object *ICreateInst::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();
  /*sophie begin:sep 19th*/
  RuleCell *rcell = tf->cur_match()->rule()->rule_cell();
  RuleInstance *rinst = rcell->find_instance(name_suffix_);
  const string temp_suffix_ = rinst->image()->name();
  /*sophie end:for maintaing image name*/

  Instance *new_inst = nullptr;
  if (model_ != nullptr) // model in the hw_lib
    new_inst = tar_cell->create_instance(
        tf->make_name(name_prefix_, temp_suffix_), model_);
  else {
    Module *model = tf->find_cell(cname_i_);
    ASSERT(model,
           TFErrMsg(TFErrMsg::UFND_ELEM) % "cell" % cname_i_.name % INSTR_NAME);
    new_inst = tar_cell->create_instance(
        tf->make_name(name_prefix_, name_suffix_), model);

    // it's bug if we use fake name to create instance that the type is not
    // slice
    tf->set_slice_inst(static_cast<PKInstance *>(new_inst));
  }
  tf->insert_new_inst(iname_i_.name, new_inst);

  // emit signal here!
  tar_cell->after_inst_created(static_cast<PKInstance *>(new_inst));

  return new_inst;
}
/*sophie begin:AUG 6TH*/
Object *ICreateIob::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *buf_inst = tf->find_instance(pname_i_);
  Net *buf_net = buf_inst->find_pin(pin_name_)->net();
  Instance::pin_iter iob_pin =
      find_if(buf_net->pins(), [](const Pin *pin) { return pin->is_mpin(); });
  const string &port_name = iob_pin->name();

  Instance *new_inst = nullptr;
  if (model_ != nullptr) // model in the hw_lib
    new_inst = tar_cell->create_instance(port_name, model_);
  else {
    Module *model = tf->find_cell(cname_i_);
    ASSERT(model,
           TFErrMsg(TFErrMsg::UFND_ELEM) % "cell" % cname_i_.name % INSTR_NAME);
    new_inst = tar_cell->create_instance(port_name, model);

    // it's bug if we use fake name to create instance that the type is not
    // slice
    tf->set_slice_inst(static_cast<PKInstance *>(new_inst));
  }
  tf->insert_new_inst(iname_i_.name, new_inst);

  // emit signal here!
  tar_cell->after_inst_created(static_cast<PKInstance *>(new_inst));

  return new_inst;
}
Object *ICreateNet::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Net *new_net =
      tar_cell->create_net(tf->make_name(name_prefix_, name_suffix_));
  tf->insert_new_net(nname_i_.name, new_net);

  // emit signal here!

  return new_net;
}

Object *ICloneCell::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  Module *model;
  if (model_ != nullptr)
    model = model_;
  else
    model = tf->find_cell(model_cname_i_);

  ASSERT(model, TFErrMsg(TFErrMsg::UFND_ELEM) % "cell" % model_cname_i_.name %
                    INSTR_NAME);

  Module *new_cell =
      model->clone(tf->make_name(name_prefix_, name_suffix_), cell_owner_);
  tf->insert_new_cell(cname_i_.name, new_cell);
  if (model_cname_i_.name == CELL_NAME::SLICE) {
    tf->set_slice_cell(static_cast<PKCell *>(new_cell));
    tf->reset_lut_num();
    tf->reset_ff_num();
  }

  return new_cell;
}

Object *IReconnect::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *old_inst = tf->find_instance(old_iname_i_);
  Instance *new_inst = tf->find_instance(new_iname_i_);
  ASSERT(old_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       old_iname_i_.name % INSTR_NAME);
  ASSERT(new_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       new_iname_i_.name % INSTR_NAME);

  PKPin *old_pin = static_cast<PKPin *>(old_inst->find_pin(old_pname_));
  PKPin *new_pin = static_cast<PKPin *>(new_inst->find_pin(new_pname_));
  ASSERT(old_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % old_pname_ % INSTR_NAME);
  ASSERT(new_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % new_pname_ % INSTR_NAME);
  ASSERT(old_pin->dir() == new_pin->dir() || new_pin->dir() == COS::INOUT,
         TFErrMsg(TFErrMsg::USAT_PINS_COND) % old_pname_ % new_pname_ %
             "the same direction" % INSTR_NAME);

  if (new_pin->is_used())
    return nullptr;
  new_pin->set_used();

  Net *net = old_pin->net();
  if (net == nullptr)
    return nullptr;

  // ASSERT(net, TFErrMsg(TFErrMsg::USAT_PIN) % "unconnected" % old_pname_ %
  // INSTR_NAME);
  ASSERT(!new_pin->net(), TFErrMsg(TFErrMsg::USAT_PIN_COND) % new_pname_ %
                              "unconnected" % INSTR_NAME);

  tf->insert_poss_dangling_inst(old_inst);
  // emit signal here!
  tar_cell->before_pin_unhook(static_cast<PKNet *>(net),
                              static_cast<PKPin *>(old_pin));
  old_pin->disconnect();

  tar_cell->before_pin_hookup(static_cast<PKNet *>(net),
                              static_cast<PKPin *>(new_pin));
  new_pin->connect(net);

  return nullptr;
}

// add by czh: begin 2009-9-17
Object *ICreateSlice::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *new_inst = nullptr;
  if (model_ != nullptr) // model in the hw_lib
    new_inst = tar_cell->create_instance(
        tf->make_name(name_prefix_, name_suffix_), model_);
  else {
    Module *model = tf->find_cell(cname_i_);
    ASSERT(model,
           TFErrMsg(TFErrMsg::UFND_ELEM) % "cell" % cname_i_.name % INSTR_NAME);
    new_inst = tar_cell->create_instance(
        tf->make_name(name_prefix_, name_suffix_), model);
  }
  // it's bug if we use fake name to create instance that the type is not slice
  // zhouxg: for removing clone instruction
  tf->set_slice_inst(static_cast<PKInstance *>(new_inst));
  tf->insert_new_inst(iname_i_.name, new_inst);

  // emit signal here!
  tar_cell->after_inst_created(static_cast<PKInstance *>(new_inst));

  return new_inst;
}
Object *IConnect::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *old_inst = tf->find_instance(old_iname_i_);
  Instance *new_inst = tf->find_instance(new_iname_i_);
  ASSERT(old_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       old_iname_i_.name % INSTR_NAME);
  ASSERT(new_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       new_iname_i_.name % INSTR_NAME);

  PKPin *old_pin = static_cast<PKPin *>(old_inst->find_pin(old_pname_));
  PKPin *new_pin = static_cast<PKPin *>(new_inst->find_pin(new_pname_));
  ASSERT(old_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % old_pname_ % INSTR_NAME);
  ASSERT(new_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % new_pname_ % INSTR_NAME);
  // ASSERT(old_pin->dir() == new_pin->dir(),
  //	TFErrMsg(TFErrMsg::USAT_PINS_COND) % old_pname_ % new_pname_
  //	% "the same direction" % INSTR_NAME);

  // tf->insert_poss_dangling_inst(old_inst);
  //// emit signal here!
  // tar_cell->before_pin_unhook(static_cast<PKNet*>(net),
  // static_cast<PKPin*>(old_pin)); old_pin->unhook();

  // tar_cell->before_pin_hookup(static_cast<PKNet*>(net),
  // static_cast<PKPin*>(new_pin)); new_pin->hookup(*net);

  string net_name = old_inst->name() + "_" + old_pin->name() + "_net_" +
                    new_inst->name() + "_" + new_pin->name();
  Net *new_net = tar_cell->create_net(net_name);
  tf->insert_new_net(net_name, new_net);

  // tar_cell->before_pin_hookup(static_cast<PKNet*>(net),
  // static_cast<PKPin*>(new_pin));
  old_pin->connect(new_net);

  // tar_cell->before_pin_hookup(static_cast<PKNet*>(net),
  // static_cast<PKPin*>(new_pin));
  new_pin->connect(new_net);

  return nullptr;
}

Object *IExconnect::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *old_inst = tf->find_instance(old_iname_i_);
  Instance *new_inst = tf->find_instance(new_iname_i_);
  ASSERT(old_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       old_iname_i_.name % INSTR_NAME);
  ASSERT(new_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       new_iname_i_.name % INSTR_NAME);

  PKPin *old_pin = static_cast<PKPin *>(old_inst->find_pin(old_pname_));
  PKPin *new_pin = static_cast<PKPin *>(new_inst->find_pin(new_pname_));
  ASSERT(old_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % old_pname_ % INSTR_NAME);
  ASSERT(new_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % new_pname_ % INSTR_NAME);
  ASSERT(old_pin->dir() == new_pin->dir(),
         TFErrMsg(TFErrMsg::USAT_PINS_COND) % old_pname_ % new_pname_ %
             "the same direction" % INSTR_NAME);

  Net *net1 = old_pin->net();
  Net *net2 = new_pin->net();

  // tf->insert_poss_dangling_inst(old_inst);
  // emit signal here!

  tar_cell->before_pin_unhook(static_cast<PKNet *>(net1),
                              static_cast<PKPin *>(old_pin));
  old_pin->disconnect();
  tar_cell->before_pin_unhook(static_cast<PKNet *>(net2),
                              static_cast<PKPin *>(new_pin));
  new_pin->disconnect();

  tar_cell->before_pin_hookup(static_cast<PKNet *>(net1),
                              static_cast<PKPin *>(new_pin));
  new_pin->connect(net1);
  tar_cell->before_pin_hookup(static_cast<PKNet *>(net2),
                              static_cast<PKPin *>(old_pin));
  old_pin->connect(net2);

  if (old_inst == new_inst &&
      new_inst->down_module()->name() == CELL_NAME::LUT) {
    string table = new_inst->property_value(INIT);
    new_inst->set_property(INIT, exchange_expr(table, old_pname_, new_pname_));
  }

  return nullptr;
}

// add by czh: end
Object *IReconnectOpt::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *old_inst = tf->find_instance(old_iname_i_);
  ASSERT(old_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       old_iname_i_.name % INSTR_NAME);
  Pin *old_pin = old_inst->find_pin(old_pname_);
  ASSERT(old_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % old_pname_ % INSTR_NAME);

  Net *net = old_pin->net();
  for (size_t i = 0; i < opt_inames_i_.size(); ++i) {

    Instance *new_inst = tf->find_instance(opt_inames_i_[i]);
    ASSERT(new_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                         opt_inames_i_[i].name % INSTR_NAME);
    PKPin *new_pin = static_cast<PKPin *>(new_inst->find_pin(opt_pnames_[i]));
    ASSERT(new_pin,
           TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % opt_pnames_[i] % INSTR_NAME);

    if (!new_pin->is_used()) {
      ASSERT(!new_pin->net(), TFErrMsg(TFErrMsg::USAT_PIN_COND) %
                                  opt_pnames_[i] % "unconnected" % INSTR_NAME);

      // emit signal here! but it always seems stop update graph here!
      if (net != nullptr) {
        tar_cell->before_pin_unhook(static_cast<PKNet *>(net),
                                    static_cast<PKPin *>(old_pin));
        old_pin->disconnect();

        tar_cell->before_pin_hookup(static_cast<PKNet *>(net),
                                    static_cast<PKPin *>(new_pin));
        new_pin->connect(net);
        /*sophie begin:AUGUST 4TH*/
        new_pin->set_used();
        /*sophie end: for counting slice used pins*/
      }
      // new_pin->set_used();

      break;
    }
  }

  return nullptr;
}
Object *IUnhook::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *inst = tf->find_instance(iname_i_);
  ASSERT(inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" % iname_i_.name %
                   INSTR_NAME);

  Pin *pin = inst->find_pin(pname_);
  ASSERT(pin, TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % pname_ % INSTR_NAME);

  tf->insert_poss_dangling_inst(inst);
  if (pin->net()) {
    tf->insert_poss_dangling_net(pin->net());
  }
  // emit signal here!
  tar_cell->before_pin_unhook(static_cast<PKNet *>(pin->net()),
                              static_cast<PKPin *>(pin));
  pin->disconnect();

  return nullptr;
}

Object *IDupConnect::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *src_inst = tf->find_instance(src_iname_i_);
  Instance *tar_inst = tf->find_instance(tar_iname_i_);
  ASSERT(src_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       src_iname_i_.name % INSTR_NAME);
  ASSERT(tar_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       tar_iname_i_.name % INSTR_NAME);

  Pin *src_pin = src_inst->find_pin(src_pname_);
  Pin *tar_pin = tar_inst->find_pin(tar_pname_);
  ASSERT(src_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % src_pname_ % INSTR_NAME);
  ASSERT(tar_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % tar_pname_ % INSTR_NAME);
  ASSERT(src_pin->dir() == tar_pin->dir(),
         TFErrMsg(TFErrMsg::USAT_PINS_COND) % src_pname_ % tar_pname_ %
             "the same direction" % INSTR_NAME);

  Net *net = src_pin->net();
  if (net == nullptr)
    return nullptr;

  // ASSERT(net, TFErrMsg(TFErrMsg::USAT_PIN) % "unconnected" % src_pname_ %
  // INSTR_NAME));
  ASSERT(!tar_pin->net(), TFErrMsg(TFErrMsg::USAT_PIN_COND) % tar_pname_ %
                              "unconnected" % INSTR_NAME);

  // emit signal here!
  tar_cell->before_pin_hookup(static_cast<PKNet *>(net),
                              static_cast<PKPin *>(tar_pin));
  tar_pin->connect(net);

  return nullptr;
}

Object *IUsePin::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  for (size_t i = 0; i < opt_inames_i_.size(); ++i) {
    Instance *inst = tf->find_instance(opt_inames_i_[i]);
    ASSERT(inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                     opt_inames_i_[i].name % INSTR_NAME);
    PKPin *pin = static_cast<PKPin *>(inst->find_pin(opt_pnames_[i]));
    ASSERT(pin,
           TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % opt_pnames_[i] % INSTR_NAME);

    if (!pin->is_used()) {
      pin->set_used();
      pin->net()->set_used(); // sophie
      static_cast<PKInstance *>(inst)->set_used();
      break;
    }
  }

  // maybe you should complain an exception when no usable pin found
  // for debugging if our algorithm work fine, but I just ignore it
  // for some special case
  return nullptr;
}

static Property<string> &find_or_create_property(Instance *inst,
                                                 const string &name) {
  Config *cfg = find_config(inst->module_type(), name);
  if (cfg)
    return *cfg;
  PropertyBase *prop = find_property(INSTANCE, name);
  if (prop) {
    Property<string> *pstr = dynamic_cast<Property<string> *>(prop);
    ASSERT(pstr, TFErrMsg(TFErrMsg::ERR_PROP_TYPE) % name);
    return *pstr;
  }
  return create_temp_property<string>(INSTANCE, name);
}

Object *ISetProperty::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  Instance *inst = tf->find_instance(iname_i_);
  ASSERT(inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" % iname_i_.name %
                   INSTR_NAME);

  map<string, string>::const_iterator prop_it = prop_map_.begin();
  while (prop_it != prop_map_.end()) {
    Property<string> &p = find_or_create_property(inst, prop_it->first);
    if (!inst->property_exist(p)) {
      inst->set_property(p, prop_it->second);
      break;
    }
    prop_it++;
  }
  return nullptr;
}

Object *ICloneProperty::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  Instance *src_inst = tf->find_instance(src_iname_i_);
  Instance *tar_inst = tf->find_instance(tar_iname_i_);
  ASSERT(src_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       src_iname_i_.name % INSTR_NAME);
  ASSERT(tar_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       tar_iname_i_.name % INSTR_NAME);

  Property<string> &p = find_or_create_property(tar_inst, prop_name_);
  // string* pval = src_inst->property_ptr(p);
  // tar_inst->set_property(p, pval ? *pval : default_value_);

  // 450
  Property<string> *prop =
      dynamic_cast<Property<string> *>(src_inst->find_property(prop_name_));
  // �޸Ĵ˴����߼� prop����Ϊpro��ֵ��������src_inst��pro���п����Ǳ��pro
  if (prop && src_inst->property_exist(*prop))
    tar_inst->set_property(p, src_inst->property_value(*prop));
  else
    tar_inst->set_property(p, default_value_);

  return nullptr;
}

Object *ICopyProperty::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  Instance *src_inst = tf->find_instance(src_iname_i_);
  Instance *tar_inst = tf->find_instance(tar_iname_i_);

  ASSERT(src_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       src_iname_i_.name % INSTR_NAME);
  ASSERT(tar_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       tar_iname_i_.name % INSTR_NAME);

  Property<string> &tar_prop =
      find_or_create_property(tar_inst, tar_prop_name_);
  string pval;
  if (src_prop_name_ == PROP_NV::NAME)
    pval = src_inst->name();
  else {
    Property<string> &sp = find_or_create_property(src_inst, src_prop_name_);
    if (!src_inst->property_exist(sp))
      return nullptr;
    pval = src_inst->property_value(sp);
  }

  if (tar_prop == INIT) // first step: lut[1-4] => lut4
    return copy_init(src_inst, tar_inst, pval);

  if (src_prop_name_.find("INIT_") == 0) {
    auto p = pval.find("'h");
    if (p != string::npos)
      pval = pval.substr(p + 2);
  }

  tar_inst->set_property(tar_prop, pval);

  return nullptr;
}

Object *ICopyProperty::copy_init(Instance *src_inst, Instance *tar_inst,
                                 const string &sinit) const {
  string newinit = "#LUT:D=";
  if (sinit[0] == '#') {
    newinit += sinit.substr(1);
  } else {
    int num = num_oldinst_inpins(src_inst);
    newinit += hex2expr(sinit, num);
  }
  tar_inst->set_property(INIT, newinit);
  tar_inst->set_property(INIT_HEX, sinit);
  return nullptr;
}

Property<deque<string>> INDEX;

Object *ISetIndex::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();

  Instance *inst = tf->find_instance(iname_i_);
  ASSERT(inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" % iname_i_.name %
                   INSTR_NAME);

  map<string, vector<string>>::const_iterator prop_it = prop_map_.begin();
  while (prop_it != prop_map_.end()) {
    if (!inst->property_exist(INDEX)) {
      deque<string> dindex;
      dindex.push_front(prop_it->second[0]);
      inst->set_property(INDEX, dindex);
    } else {
      deque<string> dindex = inst->property_value(INDEX);
      for (string s : prop_it->second) {
        if (!is_in(s, dindex)) {
          dindex.push_front(s);
          inst->set_property(INDEX, dindex);
          break;
        }
      }
    }
    prop_it++;
  }

  return nullptr;
}

Object *IShareNet::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  for (ShareCond sc : share_conds_) {
    // judge the given two pins are connected to the same net
    {
      Instance *inst1 = tf->find_instance(sc.sn_iname1_);
      PKPin *pin1 = static_cast<PKPin *>(inst1->find_pin(sc.sn_pname1_));
      Net *net1 = pin1->net();

      Instance *inst2 = tf->find_instance(sc.sn_iname2_);
      PKPin *pin2 = static_cast<PKPin *>(inst2->find_pin(sc.sn_pname2_));
      Net *net2 = pin2->net();

      if (net1 == net2)
        sc.isconnect = true;
      else
        sc.isconnect = false;
    }
    // only do the process if the two pins' nets are different
    {
      if (!sc.isconnect) {
        Instance *inst2 = tf->find_instance(sc.do_iname2_);
        PKPin *pin2 = static_cast<PKPin *>(inst2->find_pin(sc.do_pname2_));
        Net *net = pin2->net();
        if (net == nullptr)
          continue;

        Instance *inst1 = tf->find_instance(sc.do_iname1_);
        PKPin *pin1 = static_cast<PKPin *>(inst1->find_pin(sc.do_pname1_));
        if (pin1->is_used())
          continue;
        pin1->set_used();
        ASSERT(!pin1->net(), TFErrMsg(TFErrMsg::USAT_PIN_COND) % sc.do_pname1_ %
                                 "unconnected" % INSTR_NAME);

        tf->insert_poss_dangling_inst(inst2);
        tar_cell->before_pin_unhook(static_cast<PKNet *>(net),
                                    static_cast<PKPin *>(pin2));
        pin2->disconnect();

        tar_cell->before_pin_hookup(static_cast<PKNet *>(net),
                                    static_cast<PKPin *>(pin1));
        pin1->connect(net);

        if (inst2 == inst1 && inst2->down_module()->name() == CELL_NAME::LUT) {
          string table = inst2->property_value(INIT);
          inst2->set_property(INIT,
                              sub_expr(table, sc.do_pname2_, sc.do_pname1_));
        }
      }
    }
  }

  return nullptr;
}

Object *IReconnect2::execute() const {
  Transformer *tf = co_case_->co_operation()->transformer();
  PKCell *tar_cell = co_case_->co_operation()->target();

  Instance *tar_inst = tf->find_instance(old_iname_i_);
  ASSERT(tar_inst, TFErrMsg(TFErrMsg::UFND_ELEM) % "instance" %
                       old_iname_i_.name % INSTR_NAME);
  PKPin *tar_pin = static_cast<PKPin *>(tar_inst->find_pin(old_pname_));
  ASSERT(tar_pin,
         TFErrMsg(TFErrMsg::UFND_ELEM) % "pin" % old_pname_ % INSTR_NAME);
  if (tar_pin->is_used())
    return nullptr;
  tar_pin->set_used();

  for (size_t i = 0; i < opt_inames_i_.size(); i++) {
    Instance *src_inst = tf->find_instance(opt_inames_i_[i]);
    PKPin *src_pin = static_cast<PKPin *>(src_inst->find_pin(opt_pnames_[i]));
    Net *net = src_pin->net();
    if (!net)
      continue;
    tar_cell->before_pin_unhook(static_cast<PKNet *>(net),
                                static_cast<PKPin *>(src_pin));
    src_pin->disconnect();
    tar_cell->before_pin_hookup(static_cast<PKNet *>(net),
                                static_cast<PKPin *>(tar_pin));
    tar_pin->connect(net);
    break;
  }

  return nullptr;
}
} // namespace PACK