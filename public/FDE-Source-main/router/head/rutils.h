#ifndef RUTILS_H
#define RUTILS_H
#include "utils.h"

namespace FDU {
namespace RT {

using std::string;
namespace RT_CONST {
const string POSITION = "position";
const string GRM = "GRM";
const string GSB = "GSB";
} // namespace RT_CONST

#define IT_TO_PTR(x) (&(*x))
#define HUGE_FLOAT 1.e30
#define NO_PREVIOUS -1.0
#define OPEN -1

enum RTAlgorithm { BREADTH_FIRST, TIMING_DRIVEN, DIRECTED_SEARCH };
enum BaseCostType { INTRINSTIC_DELAY, DELAY_NORMALIZED, DEMAND_ONLY };

enum CostIndices {
  INVALID_INDEX = -1,
  SOURCE_COST_INDEX,
  CLK_COST_INDEX,
  SINK_COST_INDEX,
  SEGX_COST_INDEX_START
};

struct BaseCostInfo {
  double base_cost;
  double saved_base_cost;
  double inv_length;
  double TLinear;
  double TQuadratic;
  double CLoad;

  BaseCostInfo()
      : base_cost(OPEN), saved_base_cost(OPEN), inv_length(OPEN), TLinear(OPEN),
        TQuadratic(OPEN), CLoad(OPEN) {}
};

} // namespace RT
} // namespace FDU

#endif