#ifndef PLCUTILS_H
#define PLCUTILS_H

#include "log.h"
#include "utils.h"
#include <boost/format.hpp>
#include <iostream>
#include <vector>
using std::vector;

namespace FDU {
namespace Place {

using boost::format;

//////////////////////////////////////////////////////////////////////////
// preprocessor

#undef VERBOSE
// #define EXCEPTION_HANDLE
// #define CONST_GENERATE

//////////////////////////////////////////////////////////////////////////
// toolkits

template <typename F> inline void INFO(const F &info) {
  FDU_LOG(INFO) << info.str();
}

template <typename F> inline void ERR(const F &error) {
  FDU_LOG(ERR) << error.str();
}

//////////////////////////////////////////////////////////////////////////
// constants

/************************************************************************/
/*	器件信息，主要是一些特殊结构                                        */
/************************************************************************/
namespace DEVICE {
extern int NUM_SLICE_PER_TILE;
extern int NUM_CARRY_PER_TILE;
extern int NUM_LUT_INPUTS;
extern vector<vector<int>> carry_chain;
const double INV_OF_CHAN_WIDTH = 1. / 62.;
const double DEFAULT_P2P_DELAY = 0.1;

// for BLOCKRAM
const std::string LBRAMD = "LBRAMD";
const std::string RBRAMD = "RBRAMD";

// for GCLKIOB
const std::string GCLKOUT = "GCLKOUT";

// for Carry Chain
const std::string CIN = "CIN";
const std::string COUT = "COUT";
const std::string CARRY = "carry";

// for LUT6
const std::string F5 = "F5";
const std::string F5IN = "F5IN";
const std::string LUT_6 = "lut6";

// for  VCC in V2
const std::string VCCOUT = "VCCOUT";

enum DelayTableType {
  CLB_TO_CLB = 0,
  INPUT_TO_CLB,
  CLB_TO_OUTPUT,
  INPUT_TO_OUTPUT,
  NUM_DELAY_TABLE_TYPE
};
std::istream &operator>>(std::istream &s, DelayTableType &type);
std::ostream &operator<<(std::ostream &s, DelayTableType type);
} // namespace DEVICE
namespace CELL {

const string CARRY_CHAIN = "carry_chain";
const string LUT = "LUT";
const string CLUSTER = "CLB";
const string RAM = "RAM";
const string MACRO = "MACRO";
const string LE = "LE";
const string IO = "IOB";
const string GCLKBUF = "GCLK";
const string BRAM = "BRAM";
const string GCLKIOB = "GCLKIOB";
const string TRI = "TRI";
const string BUF = "BUF";
const string INV = "INV";
const string OBUF = "OBUF";
const string OBUFT = "OBUFT";
const string SLICE = "SLICE";
const string MUX2 = "MUX2";
const string RAMS16 = "RAMS16";
const string RAMD16 = "RAMD16";
const string SRLC16E = "SRLC16E";
const string SRL16E = "SRL16E";
const string IPAD = "IPAD";
const string OPAD = "OPAD";
const string ZERO = "ZERO";
const string ONE = "ONE";
const string BPAD = "BPAD";
const string DFF = "FF";
const string SDFF = "SFF";
const string XOR2 = "XOR2";
const string AND2 = "AND2";
const string LATCH = "LATCH";
const string MULTAND = "MULTAND";

// for FDP1000K
const string GRM = "GRM";
const string SITE_INFO = "site_info";
} // namespace CELL

namespace NET {
// just for placement
const string GCLK = "GCLK";
const string CLK_NET = "clock";
const string RST_NET = "reset";
const string IS_GBL = "is_gbl";
const string TYPE = "type";
const string GND = "gnd";
const string VCC = "vcc";
} // namespace NET

namespace PIN {
// just for tengine
const string TNODE = "tnode";
const string DELAY = "delay";
const string SLACK = "slack";

const string IN_FC1 = "IN_FC1";
const string OUT_TC1 = "OUT_TC1";
const string S = "S";

// just for placement
const string CY_OUT = "CY_OUT";
const string CRIT = "crit";
const string TCOST = "tcost";
const string TEMP_DELAY = "temp_delay";
const string TEMP_TCOST = "temp_tcost";
} // namespace PIN
namespace INSTANCE {
const string RLOC = "rloc";
const string SET_TYPE = "set_type";
const string POSITION = "position";
const string HSET = "h_set";
} // namespace INSTANCE

namespace LIBRARY {
const string ELEM = "element";
const string PRIM = "primitive";
const string CLUSTER = "cluster";
const string TILE = "tile";
} // namespace LIBRARY

namespace FLOORPLAN {
// stands for swap type supported by placer
//	 1. SITE refers to single-SLICE, TBUF, IOB, GCLK, GCLKIOB, BRAM, DLL
//	 2. LUT6 && CARRY_CHAIN defined as macro
enum SwapableType {
  IGNORE = -1,
  SITE,
  LUT6,
  CARRY_CHAIN,
  NUM_OF_SWAPABLE_TYPE
};

const int INVALID_VALUE = -1;
} // namespace FLOORPLAN

/************************************************************************/
/*   布局中模拟退火算法的一些参数 */
/************************************************************************/
namespace PLACER {
enum PlaceMode { BOUNDING_BOX, TIMING_DRIVEN };

// EffortLevel is used as NUM_SWAP_FACTOR,
// STANDARD(10) in VPR, MEDIUM(5) for balance, HIGH(1) for large design
enum EffortLevel { STANDARD = 10, MEDIUM = 5, HIGH = 1 };

const double DEFAULT_CRIT_EXP = 1.;
const double TIMING_TRADE_OFF = 0.5; // in VPR, this is 0.5, best is 0.5
const double HIGHEST_T = 1.0e30;

const int MAX_SWAPS_BEFORE_RECOMPUTE = 1000000;
} // namespace PLACER

//////////////////////////////////////////////////////////////////////////
// user information
/************************************************************************/
/* 控制输出信息的格式 ，具体定义在plc_utils.cpp                         */
/************************************************************************/
namespace CONSOLE {
// ERROR
extern format FILE_ERROR;
extern format PLC_ERROR;

// WARNING
extern format PLC_WARNING;

// APP INFO
extern format COPY_RIGHT;
extern format EFFORT_LEVEL;
extern format PLC_MODE;
extern format PROGRESS;
extern format FINISH;
extern format DESIGN;
extern format RSC_IN_DESIGN;
extern format DEVICE_TYPE;
extern format RSC_USAGE;
extern format RSC_SLICE;

// SA Placer INFO
extern format INIT_COST;
extern format ITER_COST;
extern format FINAL_COST;
} // namespace CONSOLE

//////////////////////////////////////////////////////////////////////////
// exceptions

// 	struct place_error : public Exception {
// 		place_error(const format& msg) throw() : Exception(msg.str()) {}
// 	};

} // namespace Place
} // namespace FDU

#endif