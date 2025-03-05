#include "utils/expCalc.h"
#include "log.h"
#include <sstream>

Exp2LUT::Exp2LUT(const std::string &expr, int bitAmount)
    : _expr(expr), _bitAmount(bitAmount) {
  _strTable.resize(_bitAmount, '0');
  _vecTable.resize(_bitAmount, 0);
  if (expr.substr(0, 2) == "0x")
    HexToTable();
  else
    expr.size() == _bitAmount &&expr.find("A") == std::string::npos ? BinaryToTable()
                                                               : ExpToTable();
}

void Exp2LUT::BinaryToTable() {
  for (int idx = 1; idx <= _bitAmount; ++idx) {
    _vecTable[_bitAmount - idx] = _expr[idx - 1] == '1' ? 1 : 0;
    _strTable[_bitAmount - idx] = _expr[idx - 1];
  }
}

void Exp2LUT::HexToTable() {
  int hex = 0;
  sscanf(_expr.c_str(), "%x", &hex);

  for (int flag = 0x8000, id = _bitAmount - 1; flag != 0; flag >>= 1, --id) {
    _vecTable[id] = hex & flag ? 1 : 0;
    _strTable[id] = hex & flag ? '1' : '0';
  }
}

void Exp2LUT::ExpToTable() {
  int varIdx = 0, maxPowIdx = 1;
  while (maxPowIdx <= _bitAmount) {
    ++varIdx;
    maxPowIdx *= 2;
  }
  maxPowIdx /= 2;

  std::vector<std::string> varNames; // names of variables
  std::vector<char> varVals(varIdx); // bool values of variables
  std::string boolExp; // expression which replaced variables with bool values

  std::stringstream ssBuf;
  std::string sBuf;
  for (int idx = 0; idx < varIdx; ++idx) {
    ssBuf.clear();
    ssBuf << idx + 1;
    ssBuf >> sBuf;
    varNames.push_back(std::string("A") + sBuf);
  }

  int powIdx;
  for (int iAddr = 0; iAddr < _bitAmount; ++iAddr) {
    boolExp = _expr;
    powIdx = maxPowIdx;
    std::string::size_type nExPos;
    for (int iAddrIndex = varIdx - 1; iAddrIndex >= 0; --iAddrIndex) {
      varVals[iAddrIndex] = static_cast<char>((iAddr / powIdx) % 2 + 48);
      powIdx /= 2;
      while ((nExPos = boolExp.find(varNames[iAddrIndex])) !=
             std::string::npos) {
        boolExp.replace(nExPos, varNames[iAddrIndex].size(), 1,
                        varVals[iAddrIndex]);
      }
    }

    std::stack<bool> tVal;
    boolCalc calc(tVal);
    ASSERTD(!boolExp.empty(), "Exp2LUT: empty expression");
    parse_info<> result = parse(boolExp.c_str(), calc, space_p);
    ASSERT(result.full, "boolCalc: invalid bool expression");
    bool val = tVal.top();
    _strTable[iAddr] = val ? '1' : '0';
    _vecTable[iAddr] = val ? 1 : 0;
  }
}