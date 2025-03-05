#include "cil/elementLib/element/pathInElem.h"
#include "log.h"

namespace FDU {
namespace cil_lib {
using namespace boost;

bool pathElem::searchMe(const std::string &in, const std::string &out) const {
  return _in == in && _out == out;
}

void pathElem::listSrams(vecBits &bits) { _cfgInfo.listSrams(bits); }

void contPathsElem::listPathSrams(const pathInfo &path, vecBits &bits) {
  pathElemIter it = boost::find_if(paths(), [&path](const pathElem *pe) {
    return pe->searchMe(path._in, path._out);
  });
  ASSERTD(it != _paths.end(), "Element: no such path ... " + path.toStr());
  it->listSrams(bits);
}

bool contPathsElem::hasRequiredPath(const std::string &in,
                                    const std::string &out) {
  pathElemIter it = boost::find_if(paths(), [&in, &out](const pathElem *pe) {
    return pe->searchMe(in, out);
  });
  return it != _paths.end();
}

} // namespace cil_lib
} // namespace FDU