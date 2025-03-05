#ifndef _CKTBASE_H_
#define _CKTBASE_H_

#include "object.hpp"

#include <boost/noncopyable.hpp>
#include <boost/regex.hpp>
#include <string>

namespace BitGen {
namespace circuit {
using namespace FDU;

class cktBase : boost::noncopyable {
public:
  using xdlMatch = boost::match_results<std::string::const_iterator>;

protected:
  std::string _name;
  COS::Object *_nlBase;
  xdlMatch *_matchResults;

public:
  // for XDL
  cktBase(const std::string &name, xdlMatch *matchResults = 0)
      : _name(name), _nlBase(nullptr), _matchResults(matchResults) {}
  // for XML
  cktBase(COS::Object *nlBase = 0)
      : _name(""), _nlBase(nlBase), _matchResults(nullptr) {}

  std::string getName() const { return _name; }
  std::string name() const { return _name; }
  void setName(const std::string &name) { _name = name; }

  //		virtual void constructFromXDL(const std::string& info = "") = 0;
  //		virtual void constructFromXML() = 0;
};

} // namespace circuit
} // namespace BitGen

#endif