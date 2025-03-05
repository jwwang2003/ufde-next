#ifndef _RTMSG_H
#define _RTMSG_H

#include <boost/format.hpp>
#include <ostream>
#include <string>

using ccstr = const char *;

namespace FDU {
namespace RT {

using std::string;

class MsgFormat {
public:
  MsgFormat(const string &format_str = "", const string &msg_head = "")
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
  string msg_head_;
  mutable string formatted_;
};

inline std::ostream &operator<<(std::ostream &out, const MsgFormat &err_msg) {
  out << err_msg.to_string();
  return out;
}

const char *const err_msg[] = {
    "[COMMAND ERROR]: missing %1% file",               // CMDERR_FILE_MISS
    "[COMMAND ERROR]: invalid output %1% file format", // CMDERR_ILL_FILE_FMT
    "[FILE ERROR]: cannot open file \"%1%\"",          // FILERR_FAIL_OPEN_FILE
    "[NETLIST ERROR]: net \"%1%\" have no sink",       // NLTERR_NO_SNK
    "[NETLIST ERROR]: net \"%1%\" must be single source", // NLTERR_NOT_SGL_SRC
    "[NETLIST ERROR]: the to_net\"%1%\" of last pip in net \"%2%\" does not "
    "connect a sink",                             // NLTERR_LAST_PIP_ILLEGAL
    "[ROUTING ERROR]: instance \"%1%\" unplaced", // RTERR_INST_UPLC
    "[ROUTING ERROR]: switch from \"%1%\" to \"%2%\" not exist", // RTERR_UFND_SWH
    "[ROUTING ERROR]: unfound %1% \"%2%\" of net \"%3%\", please check the "
    "name or direction of the pin",            // RTERR_UFND_NODE
    "[ROUTING ERROR]: unroutable net \"%1%\"", // RTERR_UNRT_NET
    "[ROUTING ERROR]: congestion unresolved, the design seems unroutable", // RTERR_CONG_UNRESL
    "Error(s) occurred when routing. Stop routing!",        // RTERR_ST_RT
    "[ROUTING ERROR]: there are branches in the new path!", // RTERR_LD_RT
    "[ROUTING ERROR]: invalid position of %1% \"%2%\"",     // RTERR_POS
    "[ROUTING ERROR]: unknown type of rrg node \"%1%\", from %2% to %3%", // RTERR_NODE_TYPE
    "[ROUTING ERROR]: error number of pins for net \"%1%\"", // ERROR_NUM_PINS
    "[ROUTING ERROR]: can not find sink pin \"%1%\" for net \"%2\"", // ERROR_FIND_SINK_PIN
    "[ROUTING ERROR]: unknown config type between \"%1%\" and \"%2%\"" // RTERR_NODE_CONF
};

class ErrMsg : public MsgFormat {
public:
  enum ErrMsgType {
    CMDERR_FILE_MISS,
    CMDERR_ILL_FILE_FMT,
    FILERR_FAIL_OPEN_FILE,
    NLTERR_NO_SNK,
    NLTERR_NOT_SGL_SRC,
    NLTERR_LAST_PIP_ILLEGAL,
    RTERR_INST_UPLC,
    RTERR_UFND_SWH,
    RTERR_UFND_NODE,
    RTERR_UNRT_NET,
    RTERR_CONG_UNRESL,
    RTERR_ST_RT,
    RTERR_LD_RT,
    RTERR_POS,
    RTERR_NODE_TYPE,
    ERROR_NUM_PINS,
    ERROR_FIND_SINK_PIN,
    RTERR_NODE_CONF
  };

  ErrMsg(ErrMsgType emsg_type, const string &msg_head = "")
      : MsgFormat(err_msg[emsg_type], msg_head) {}
};
const char *const waring_msg[] = {
    "[NETLIST WARNING] : cell port \"%1%\" connect to net \"%2%\"", // INFO_PORT_CONN
    "[NETLIST WARNING] : there is no pins in net \"%1%\"", // INFO_PORT_CONN
    "[NETLIST WARNING] : net \"%1%\" is given partial routed", // INFO_PARTIAL_ROUTED
    "[COMMAND WARNING] : You have specified astar but are not intended to use directed-search algorithm,\
							using directed-search algorithm anyway..." // CMDWNG_AMBIGUITY_ALGORITHM
};

class WarMsg : public MsgFormat {
public:
  enum WarMsgType {
    NLTWNG_PORT_CONN,
    NLTWNG_NO_PINS,
    NLTWNG_PARTIAL_ROUTED,
    CMDWNG_AMBIGUITY_ALGORITHM
  };
  WarMsg(WarMsgType wmsg_type, const string &msg_head = "")
      : MsgFormat(waring_msg[wmsg_type], msg_head) {}
};

const char *const info_msg[] = {
    "Release 2.0 - route\n"
    "Copyright (c) 2006-2010 FPGA-SoftwareGP.MEKeylab.FDU   All rights "
    "reserved.",                                   // INFO_CPY_RGHT
    "Progress   0%%: parsing command...",          // INFO_PAR_CMD
    "Progress   5%%: loading netlist\"%1%\"...",   // INFO_LOAD_NL
    "Progress  10%%: loading arch file\"%1%\"...", // INFO_LOAD_ARCH
    "Progress  20%%: checking netlist...",         // INFO_CHK_NL
    "Progress  30%%: building routing internal data struct...", // INFO_BUILD_DS
    "Progress  50%%: trying to route the design using %1% algorithm...", // INFO_RT_DESIGN
    "Progress 100%%: exporting result to \"%1%\"...", // INFO_EXPT_RST
    "Sim File: Exporting Sim file to \"%1%\"...",     // INFO_SIM_FILE
    "Design: \"%1%\", resource statistic:",           // INFO_ECHO_DESIGN
    "ChipType: \"%1%\"",                              // INFO_ECHO_CHIP
    "Total routing resource:",                        // INFO_ECHO_RESOURCE
    "  * Num of %1%: %2%",                            // INFO_ECHO_RSRC
    "  * After %1% iterations %2% net(s) unrouted",   // INFO_ECHO_NUM_URT_NET
    "Successfully route the design after %1% iterations. Pips Usage: %2%%%. "
    "Number of ignored nets: %3%",       // INFO_SUCES_RT
    "Successfully route the design. Elapsed Time: %1%s" // INFO_FNL_RT
};

class InfoMsg : public MsgFormat {
public:
  enum InfoMsgType {
    INFO_CPY_RGHT,
    INFO_PAR_CMD,
    INFO_LOAD_NL,
    INFO_LOAD_ARCH,
    INFO_CHK_NL,
    INFO_BUILD_DS,
    INFO_RT_DESIGN,
    INFO_EXPT_RST,
    INFO_SIM_FILE,
    INFO_ECHO_DESIGN,
    INFO_ECHO_CHIP,
    INFO_ECHO_RESOURCE,
    INFO_ECHO_RSRC,
    INFO_ECHO_NUM_URT_NET,
    INFO_SUCES_RT,
    INFO_FNL_RT
  };

  InfoMsg(InfoMsgType emsg_type, const string &msg_head = "")
      : MsgFormat(info_msg[emsg_type], msg_head) {}
};

} // namespace RT
} // namespace FDU

#endif