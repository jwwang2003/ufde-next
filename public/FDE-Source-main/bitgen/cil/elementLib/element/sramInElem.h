#ifndef _SRAMINELEM_H_
#define _SRAMINELEM_H_

#include "container/Container.h"
#include "utils/cfgInTile.h"

namespace FDU {
namespace cil_lib {

class sramElem : public CilBase {
private:
  bool _default;
  int _content;

public:
  sramElem(bool defaultORnot, int content, const std::string &sramName)
      : CilBase(sramName), _default(defaultORnot), _content(content) {}

  void setContent(int content) { _content = content; }
  int getContent() const { return _content; }
  void saveTo(vecBits &bits);
};

class contSramsElem {
public:
  using sramsElemType = cilContainer<sramElem>::range_type;
  using const_sramsElemType = cilContainer<sramElem>::const_range_type;
  using sramElemIter = cilContainer<sramElem>::iterator;
  using const_sramElemIter = cilContainer<sramElem>::const_iterator;

private:
  cilContainer<sramElem> _srams;

public:
  sramsElemType srams() { return _srams.range(); }
  const_sramsElemType srams() const { return _srams.range(); }

  sramElem *addSram(sramElem *sram) { return _srams.add(sram); }
  sramElem &getSram(const std::string &sramName) {
    return *srams().find(sramName);
  }

  void listSrams(vecBits &bits);
  // void listCktSrams(vecBits& bits);
};

} // namespace cil_lib
} // namespace FDU

#endif