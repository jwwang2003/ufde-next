#ifndef _XMLCKTHANDLER_H_
#define _XMLCKTHANDLER_H_

#include "circuit/cktHandler.h"
#include "netlist.hpp"

namespace BitGen {
namespace circuit {
using namespace FDU;

class xmlCktHandler : public cktHandler {
private:
  COS::Design _design;

public:
  explicit xmlCktHandler(Circuit *ckt) : cktHandler(ckt) {}

  virtual void parse(const std::string &file);
};

} // namespace circuit
} // namespace BitGen

#endif