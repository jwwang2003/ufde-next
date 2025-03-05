#ifndef _PATHINELEM_H_
#define _PATHINELEM_H_

#include "cil/elementLib/element/sramInElem.h"

namespace FDU {
namespace cil_lib {

class pathElem : public CilBase {
private:
  std::string _in;
  std::string _out;
  bool _segregated;
  contSramsElem _cfgInfo;

public:
  using cstString = const std::string;
  pathElem(cstString &pathName, cstString &in, cstString &out, bool seg = false)
      : CilBase(pathName), _in(in), _out(out), _segregated(seg) {}

  void setSegregated(bool Segregated) { _segregated = Segregated; }
  bool isSegregated() const { return _segregated; }
  bool searchMe(const std::string &in, const std::string &out) const;

  sramElem *addCfgSram(sramElem *sram) { return _cfgInfo.addSram(sram); }
  sramElem &getCfgSram(const std::string &sramName) {
    return _cfgInfo.getSram(sramName);
  }

  contSramsElem &getCfgInfo() { return _cfgInfo; }

  void listSrams(vecBits &bits);
};

class contPathsElem {
public:
  using pathsElemType = cilContainer<pathElem>::range_type;
  using const_pathsElemType = cilContainer<pathElem>::const_range_type;
  using pathElemIter = cilContainer<pathElem>::iterator;
  using const_pathElemIter = cilContainer<pathElem>::const_iterator;

private:
  cilContainer<pathElem> _paths;

public:
  pathsElemType paths() { return _paths.range(); }
  const_pathsElemType paths() const { return _paths.range(); }

  pathElem *addPath(pathElem *path) { return _paths.add(path); }
  pathElem &getPath(const std::string &pathName) {
    return *paths().find(pathName);
  }

  void listPathSrams(const pathInfo &path, vecBits &bits);
  bool hasRequiredPath(const std::string &in, const std::string &out);
};

} // namespace cil_lib
} // namespace FDU

#endif