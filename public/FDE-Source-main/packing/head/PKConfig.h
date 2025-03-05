#ifndef _PKCONFIG_H
#define _PKCONFIG_H

#include "netlist.hpp"
#include <iosfwd>

namespace PACK {

#define SLICE_LAYER 1

#define LUT_SIZE_PER_SLICE 2
#define FF_SIZE_PER_SLICE 2
#define INVALID -1

using std::string;

namespace VCELL_ATTR {
enum VCELL_TYPE {
  BRAM = 0,
  DRAM,
  MULTIPLIER,
  CARRY_CHAIN,
  LUT6,
  LUT5,
  SLICE,
  IOB,
  BUFGMUX,
  GCLKIOB, // ZZZ��BUFGMUX must be before GCLKIOB
  TBUF,
  DLL
};
}

namespace KEY_WORD {
const std::string RULE_CELL = "RULE_CELL";
const std::string SRC_NODE = "SRC_NODE";
const std::string TOP_CELL = "TopCell";
const std::string SLICE_CELL = "slice_cell";
const std::string SLICE_INST = "slice_inst";
} // namespace KEY_WORD

namespace CELL_NAME {
const std::string LUT = "LUT";
const std::string FF = "FF";
const std::string SLICE = "slice";
} // namespace CELL_NAME

namespace PIN_NAME {
const std::string CLK = "CLK";
const std::string CE = "CE";
const std::string SR = "SR";
const std::string CIN = "CIN";
const std::string COUT = "COUT";
const std::string BX = "BX";
const std::string XB = "XB";
const std::string YB = "YB";
} // namespace PIN_NAME

namespace PROP_NV {
const std::string INIT = "INIT";
// const std::string TRUTHTABLE  = "TRUTHTABLE";
const std::string NAME = "NAME";
const std::string ONE = "#1";
const std::string ZERO = "#0";
const std::string CKINV = "CKINV";
const std::string CEMUX = "CEMUX";
const std::string SRMUX = "SRMUX";
const std::string OFF = "#OFF";
} // namespace PROP_NV

namespace CFG_NAME {
const string DEF_VALUE = "#OFF";
const string DEF_INIT('0', 64); // BRAM INIT

const string SLICE = "SLICE";
const string IOB = "IOB";
const string GCLKIOB = "GCLKIOB";
const string GCLK = "GCLK";
const string BLOCKRAM = "BLOCKRAM";
const string DANGLEIOB = "DANGLEIOB";
const string TBUF = "TBUF";
const string BUFGMUX = "BUFGMUX";
const string DLL = "DLL";

//		enum CFG_TYPE { SLICE, IOB, GCLKIOB, GCLK, BLOCKRAM, TBUF,
// BUFGMUX, DLL, NUM_CFGS };
} // namespace CFG_NAME

namespace CHAIN_NAME {
const string ADDF = "ADDF";
const string ADDF_A = "A";
const string ADDF_B = "B";
const string ADDF_CI = "CI";
const string ADDF_CO = "CO";
const string ADDF_S = "S";
const string LUT = "LUT";
const string LUT_O = "D";
const string CYMUX = "CYMUX";
const string CYMUX_DI = "I0";
const string CYMUX_CI = "I1";
const string CYMUX_S = "S";
const string CYMUX_O = "OUT";
const string XOR = "XOR";
const string XOR_DI = "A";
const string XOR_CI = "B";
const string XOR_O = "O";
} // namespace CHAIN_NAME

namespace CHIP_TYPE_NAME {
const string DEF_VALUE = "UNKNOWN_CHIP_TYPE";
const string FDP3 = "fdp3"; // 1000k,3000k
const string FDP4 = "fdp4"; // 8����
extern string chip_type_;
} // namespace CHIP_TYPE_NAME

using namespace CHIP_TYPE_NAME;

class PKConfig {
protected:
  COS::Config &config_;
  string xdl_name_;
  bool has_inner_;

public:
  PKConfig(const string &inst_type, const string &name, const string &xdl_name,
           bool has_inner = false, const string &def_val = CFG_NAME::DEF_VALUE);

  COS::Config &cos_cfg() { return config_; }
  const string &name() const { return config_.name(); }
  void xdl_out(std::ostream &os, COS::Instance *inst);
};

class ConfigInfo {
public:
  enum cfg_type { NORMAL, LUT, RAM_INIT };
  struct value_type { // for supporting boost assign
    cfg_type type_;
    string name_;
    value_type(const char *name, cfg_type type = NORMAL)
        : type_(type), name_(name) {}
    value_type(const string &name, cfg_type type = NORMAL)
        : type_(type), name_(name) {}
  };
  typedef COS::PtrVector<PKConfig> configs_type;
  const configs_type &get_configs(const string &inst_type) {
    return config_map_[inst_type];
  }

  // void init();
  void insert(const value_type &val);
  void load(const string &fg); // added by hz
  string using_type(const string &inst_type) {
    return cur_type_ = inst_type;
  } // modified by hz

private:
  // void using_type(const string& inst_type) { cur_type_ = inst_type; }

  std::map<string, configs_type> config_map_;
  string cur_type_;
};
} // namespace PACK

#endif