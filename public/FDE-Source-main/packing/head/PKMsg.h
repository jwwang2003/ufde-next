#ifndef _PACK_ERR_MSG_H
#define _PACK_ERR_MSG_H

#include "boost/format.hpp"
#include <ostream>

using ccstr = const char *;

namespace PACK {

class MsgFormat {
public:
  MsgFormat(const std::string &format_str = "",
            const std::string &msg_head = "")
      : formator_(format_str), msg_head_(msg_head) {}

  template <class T> MsgFormat &operator%(const T &parm) {
    formator_ % parm;
    return *this;
  }
  // conversion operator:convert MsgFormat to a string
  const std::string &to_string() const {
    formatted_ = msg_head_ + formator_.str();
    return formatted_;
  }
  operator ccstr() const { return to_string().c_str(); }

protected:
  boost::format formator_; // cannot be accessed by the user of MsgFormat, thus
                           // we overload %
private:
  std::string msg_head_;
  mutable std::string formatted_;
};
////////////////////////sophie/////////////////////////////////////
inline std::ostream &operator<<(std::ostream &os, const MsgFormat &msg_fmt) {
  os << msg_fmt.to_string();
  return os;
}
// inline std::ostream& operator<< (std::ostream& out, MsgFormat& err_msg) { out
// << std::string(err_msg).c_str(); return out; }
//  parsing operation error message
const char *const poerr_msg[] = {
    "invalid statement \"%1%\"",
    "unknown instruction \"%1%\"",
    "\"%1%\" expects %2% arguments - %3% provided",
    "\"%1%\" expects %2% or more arguments - %3% provided",
    "unexpected argument format \"%1%\" of \"%2%\"",
    "%1% \"%2%\" not found in instruction \"%3%\"",
    "unknown test expression \"%1%\""};

class POErrMsg : public MsgFormat {
public:
  enum POErr {
    IVLD_STATE,
    UKWN_INSTR,
    UEXP_ARG_NUM,
    UEXP_ARG_NUM_MORE,
    UEXP_ARG_FMT,
    UFND_ELEM,
    UKWN_TEXPR
  };

  POErrMsg(POErr emsg_type, const std::string &msg_head = "[OP ERROR]: ")
      : MsgFormat(poerr_msg[emsg_type], msg_head) {}

  using MsgFormat::operator%;
};

// transform circuit error message
const char *const tferr_msg[] = {
    "%1% \"%2%\" not found when execute \"%3%\"",
    "pin \"%1%\" must be %2% when execute \"%3%\"",
    "pin \"%1%\" and \"%2%\" must be %3% when execute \"%4%\"",
    "\"%1%\": wrong property type"};

class TFErrMsg : public MsgFormat {
public:
  enum TFErr { UFND_ELEM, USAT_PIN_COND, USAT_PINS_COND, ERR_PROP_TYPE };

  TFErrMsg(TFErr emsg_type, const std::string &msg_head = "[TF ERROR]: ")
      : MsgFormat(tferr_msg[emsg_type], msg_head) {}

  using MsgFormat::operator%; // uncomment
};

const char *const info_msg[] = {
    "Release 1.0 - CSP_Packing\n"
    "Copyright (c) 2006-2011 FPGA-SoftwareGP.MEKeylab.FDU   All rights "
    "reserved.",                                              // INFO_CPY_RGHT
    "Progress    0%%: parsing command...\n",                  // INFO_PAR_CMD
    "Progress    5%%: loading cell_library \"%1%\"...\n",     // INFO_LOAD_CLIB
    "Progress    8%%: loading netlist \"%1%\"...\n",          // INFO_LOAD_NL
    "Progress   10%%: loading pattern library \"%1%\" ...\n", // INFO_LOAD_PLIB
    "Progress   15%%: begin to map the netlist to hardware "
    "primittives...\n",                                         // INFO_MAP2HW
    "Progress   40%%: begin to preprocess macro...\n",          // INFO_PREMACRO
    "Progress   60%%: begin to map the netlist to macros...\n", // INFO_MAP2MACRO
    "Progress   80%%: begin to map the netlist to clusters...\n", // INFO_MAP2CLUSTER
    "Successfully pack the design. Elapsed Time: %1%s" // INFO_PACK_FINISH
};
class InfoMsg : public MsgFormat {
public:
  enum InfoMsgType {
    INFO_CPY_RGHT,
    INFO_PARSE_CMD,
    INFO_LOAD_CLIB,
    INFO_LOAD_NL,
    INFO_LOAD_PLIB,
    INFO_MAP2HW,
    INFO_PREMACRO,
    INFO_MAP2MACRO,
    INFO_MAP2CLUSTER,
    INFO_PACK_FINISH

  };
  InfoMsg(InfoMsgType msg_type, const std::string &msg_head = "")
      : MsgFormat(info_msg[msg_type], msg_head) {}
};

} // namespace PACK
#endif