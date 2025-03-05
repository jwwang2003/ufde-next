#ifndef _XDLCKTHANDLER_H_
#define _XDLCKTHANDLER_H_

#include "circuit/cktBase.h"
#include "circuit/cktHandler.h"

#include <boost/regex.hpp>
#include <istream>

namespace BitGen {
namespace circuit {

class xdlCktHandler : public cktHandler {
private:
  std::string _contents;

public:
  explicit xdlCktHandler(Circuit *ckt) : cktHandler(ckt) {}

  void loadXDL(std::istream &xdl);
  void regulateContents();

  virtual void parse(const std::string &file);
};

//////////////////////////////////////////////////////////////////////////
// function object for parsing

class parseObject {
private:
  Circuit *_curCkt;

public:
  explicit parseObject(Circuit *curCkt) : _curCkt(curCkt) {}

  bool operator()(const cktBase::xdlMatch &what) const;
};

} // namespace circuit
} // namespace BitGen

#endif