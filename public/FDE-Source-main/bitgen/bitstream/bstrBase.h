#ifndef _BSTRBASE_H_
#define _BSTRBASE_H_

#include <boost/noncopyable.hpp>
#include <string>

namespace BitGen {
namespace bitstream {

class bstrBase : boost::noncopyable {
protected:
  std::string _name;

public:
  explicit bstrBase(const std::string &name) : _name(name) {}

  std::string getName() const { return _name; }
  std::string name() const { return _name; }
  void setName(const std::string &name) { _name = name; }

  virtual void construct() = 0;
};

} // namespace bitstream
} // namespace BitGen

#endif