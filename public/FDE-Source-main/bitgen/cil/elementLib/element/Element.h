#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include "cil/elementLib/element/pathInElem.h"

namespace FDU {
namespace cil_lib {

class Element : public CilBase {
public:
  using Srams = contSramsElem;
  using Paths = contPathsElem;

private:
  Srams _srams;
  Paths _paths;

public:
  Element(const std::string &elemName) : CilBase(elemName) {}

  Srams &getSrams() { return _srams; }
  Paths &getPaths() { return _paths; }

  void listDfotSrams(vecBits &bits);

  bool hasRequiredPath(const std::string &in, const std::string &out);
  void listPathSrams(const pathInfo &path, vecBits &bits);
};

inline void Element::listDfotSrams(vecBits &bits) { _srams.listSrams(bits); }

inline bool Element::hasRequiredPath(const std::string &in,
                                     const std::string &out) {
  return _paths.hasRequiredPath(in, out);
}

inline void Element::listPathSrams(const pathInfo &path, vecBits &bits) {
  _paths.listPathSrams(path, bits);
}

} // namespace cil_lib
} // namespace FDU

#endif