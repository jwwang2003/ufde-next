#include "circuit/netLib/net/pinInNet.h"
#include "exception/exceptions.h"
#include "log.h"

#include <boost/regex.hpp>

namespace BitGen {
namespace circuit {

void pinNet::constructFromXDL(const std::string &info) {
  static const char *pinExp =
      // possibly leading whitespace:
      "\\s*"
      // pin information: [1] = instName, [2] = pinName
      "\"(.+)\"\\s*(.+)\\s*";
  static boost::regex pinRegex(pinExp);

  boost::smatch match;
  ASSERT(boost::regex_match(info, match, pinRegex),
         "pin: invalid pin in xdl ... " + info);
  _instName = match[1].str();
  _name = match[2].str();
}

} // namespace circuit
} // namespace BitGen