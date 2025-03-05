#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include "PKUtils.h"

namespace PACK {

class TranTestCase;

enum InstrucType {
  UKWN_INSTR = -1,
  CREATE_INST // sophie
  ,
  CREATE_SLICE // czh
  ,
  CREATE_IOB,
  CREATE_NET,
  CLONE_CELL,
  RECONNECT,
  RECONNECT2,
  CONNECT // czh
  ,
  EXCONNECT // czh
  ,
  RECONNECT_OPT,
  UNHOOK,
  DUPCONNECT,
  USE_PIN,
  SET_PROPERTY,
  CLONE_PROPERTY,
  COPY_PROPERTY // sohpie
  ,
  SET_INDEX,
  SHARE_NET,
  NUM_INSTRUCTS
};

std::istream &operator>>(std::istream &s, InstrucType &itype);

class InstructParser {
  typedef void (*PIHandler)(const string &, TranTestCase *);

public:
  static void parse_instruct(const string &, TranTestCase *);
  static void parse_name_info(NameInfo &name_i, const string &name,
                              const string &key_word) {
    name_i.name = name;
    if (name == key_word)
      name_i.is_kw = true;
    else
      name_i.is_fake = is_fake_name(name);
  }

public:
  static PIHandler parsing_array[NUM_INSTRUCTS];
  static boost::smatch regex_m;
};

class Instruct {
  friend class InstructParser;

public:
  virtual COS::Object *execute() const = 0;
  virtual c_str_ptr name() const = 0;

  virtual ~Instruct() {}

protected:
  Instruct(TranTestCase *c) : co_case_(c) {}
  // PKPin* find_pin(const NameInfo&, const string&);

protected:
  TranTestCase *co_case_;
};

class ICreateInst : public Instruct {
  friend class InstructParser;

public:
  ICreateInst(TranTestCase *c) : Instruct(c), model_(nullptr) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo iname_i_;
  string name_prefix_;
  string name_suffix_;

  NameInfo cname_i_;
  COS::Module *model_; // from which the correspond inst is created

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};
/*sophie begin: AUG 6TH*/
class ICreateIob : public Instruct {
  friend class InstructParser;

public:
  ICreateIob(TranTestCase *c) : Instruct(c), model_(nullptr) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo iname_i_;
  string name_prefix_;
  string name_suffix_;
  NameInfo cname_i_;
  NameInfo pname_i_;
  string pin_name_;
  COS::Module *model_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
}; /*sophie end:for maintaining port names of iob*/

class ICreateNet : public Instruct {
  friend class InstructParser;

public:
  ICreateNet(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo nname_i_;
  string name_prefix_;
  string name_suffix_;
  // string net_type_;       //need to further implementation

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class ICloneCell : public Instruct {
  friend class InstructParser;

public:
  ICloneCell(TranTestCase *c) : Instruct(c), model_(nullptr) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo cname_i_;
  string name_prefix_;
  string name_suffix_;

  NameInfo model_cname_i_;
  COS::Module *model_;
  COS::Library *cell_owner_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class IReconnect : public Instruct {
  friend class InstructParser;

public:
  IReconnect(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo old_iname_i_;
  NameInfo new_iname_i_;

  string old_pname_;
  string new_pname_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class IReconnectOpt : public Instruct {
  friend class InstructParser;

public:
  IReconnectOpt(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo old_iname_i_;
  string old_pname_;

  std::vector<NameInfo> opt_inames_i_;
  std::vector<string> opt_pnames_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class IUnhook : public Instruct {
  friend class InstructParser;

public:
  IUnhook(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo iname_i_;
  string pname_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class IDupConnect : public Instruct {
  friend class InstructParser;

public:
  IDupConnect(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo src_iname_i_;
  NameInfo tar_iname_i_;

  string src_pname_;
  string tar_pname_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class IUsePin : public Instruct {
  friend class InstructParser;

public:
  IUsePin(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  std::vector<NameInfo> opt_inames_i_;
  std::vector<string> opt_pnames_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class ISetProperty : public Instruct {
  friend class InstructParser;

public:
  ISetProperty(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo iname_i_;
  std::map<string, string> prop_map_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class ICloneProperty : public Instruct {
  friend class InstructParser;

public:
  ICloneProperty(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo src_iname_i_;
  NameInfo tar_iname_i_;
  string prop_name_;
  string default_value_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

//////sophie///////////////////////////
class ICopyProperty : public Instruct {
  friend class InstructParser;

public:
  ICopyProperty(TranTestCase *c) : Instruct(c) {}
  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);
  COS::Object *copy_init(COS::Instance *src_inst, COS::Instance *tar_inst,
                    const string &sinit) const;

  NameInfo src_iname_i_;
  NameInfo tar_iname_i_;
  string src_prop_name_;
  string tar_prop_name_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};
//////sophie///////////////////////////

// add by czh: begin 2009-9-17
class ICreateSlice : public Instruct {
  friend class InstructParser;

public:
  ICreateSlice(TranTestCase *c) : Instruct(c), model_(nullptr) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo iname_i_;
  string name_prefix_;
  string name_suffix_;

  NameInfo cname_i_;
  COS::Module *model_; // from which the correspond inst is created

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};
class IConnect : public Instruct {
  friend class InstructParser;

public:
  IConnect(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo old_iname_i_;
  NameInfo new_iname_i_;

  string old_pname_;
  string new_pname_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};
class IExconnect : public Instruct {
  friend class InstructParser;

public:
  IExconnect(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo old_iname_i_;
  NameInfo new_iname_i_;

  string old_pname_;
  string new_pname_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class ISetIndex : public Instruct {
  friend class InstructParser;

public:
  ISetIndex(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo iname_i_;
  std::map<string, std::vector<string>> prop_map_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};
class IShareNet : public Instruct {
  friend class InstructParser;

public:
  IShareNet(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  struct ShareCond {
    NameInfo sn_iname1_;
    NameInfo sn_iname2_;
    string sn_pname1_;
    string sn_pname2_;

    NameInfo do_iname1_;
    NameInfo do_iname2_;
    string do_pname1_;
    string do_pname2_;

    bool isconnect;
  };

  std::vector<ShareCond> share_conds_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

class IReconnect2 : public Instruct {
  friend class InstructParser;

public:
  IReconnect2(TranTestCase *c) : Instruct(c) {}

  c_str_ptr name() const { return INSTR_NAME; }
  COS::Object *execute() const;

private:
  static void parse(const string &, TranTestCase *);

private:
  NameInfo old_iname_i_;
  string old_pname_;

  std::vector<NameInfo> opt_inames_i_;
  std::vector<string> opt_pnames_;

public:
  static const int NUM_PARAM;
  static const char *const INSTR_NAME;
};

// add by czh: end
} // namespace PACK

#endif