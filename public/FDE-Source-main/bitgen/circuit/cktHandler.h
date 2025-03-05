#ifndef _CKTHANDLER_H_
#define _CKTHANDLER_H_

#include "circuit/circuit.h"

#include <string>

namespace BitGen {
namespace circuit {

class cktHandler {
protected:
  Circuit *_curCkt;

public:
  explicit cktHandler(Circuit *curCkt) : _curCkt(curCkt) {}

  virtual void parse(const std::string &file) = 0;
};

} // namespace circuit
} // namespace BitGen

#endif