#ifndef _CFGINSITE_H_
#define _CFGINSITE_H_

#include <vector>

#include "cil/elementLib/elemLib.h"
#include "utils/cfgInTile.h"

namespace FDU {
namespace cil_lib {

//////////////////////////////////////////////////////////////////////////////////////////
/// sram_namespace in config_info

struct bCellInSramNameSite {
  std::string _name;
  std::string _sram;
  elemLib *_refElemLib;

  bCellInSramNameSite(const std::string &name, const std::string &sram,
                      elemLib *refElemLib = 0)
      : _name(name), _sram(sram), _refElemLib(refElemLib) {}
};

class sramNameInNameSpSite : public CilBase {
public:
  using BasicCells = cilContainer<bCellInSramNameSite>;

private:
  BasicCells _basicCells;
  elemLib *_refElemLib;

public:
  sramNameInNameSpSite(const std::string &name, elemLib *refElemLib = 0)
      : CilBase(name), _refElemLib(refElemLib) {}

  void setRefElemLib(elemLib *ref) { _refElemLib = ref; }
  elemLib *getRefElemLib() const { return _refElemLib; }

  bCellInSramNameSite *addBasicCell(bCellInSramNameSite *basicCell) {
    return _basicCells.add(basicCell);
  }
  BasicCells &getBasicCells() { return _basicCells; }
};

class sramNameSpSite {
public:
  using sramNamesType = cilContainer<sramNameInNameSpSite>::range_type;
  using const_sramNamesType =
      cilContainer<sramNameInNameSpSite>::const_range_type;
  using sramNameIter = cilContainer<sramNameInNameSpSite>::iterator;
  using const_sramNameIter = cilContainer<sramNameInNameSpSite>::const_iterator;

private:
  cilContainer<sramNameInNameSpSite> _sramNames;
  elemLib *_refElemLib;

public:
  explicit sramNameSpSite(elemLib *refElemLib = 0) : _refElemLib(refElemLib) {}

  void setRefElemLib(elemLib *ref) { _refElemLib = ref; }
  elemLib *getRefElemLib() const { return _refElemLib; }

  sramNamesType sramNames() { return _sramNames.range(); }
  const_sramNamesType sramNames() const { return _sramNames.range(); }

  sramNameInNameSpSite *addSramName(sramNameInNameSpSite *sramName) {
    return _sramNames.add(sramName);
  }
  sramNameInNameSpSite &getSramName(const std::string &sramName) {
    return *sramNames().find(sramName);
  }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// cfg_element in config_info

struct sramInCfgElemSite {
  std::string _basicCell;
  std::string _name;
  int _content;
  int _optAddr;
  elemLib *_refElemLib;
  sramNameSpSite *_refSramNameSp;

  sramInCfgElemSite(const std::string &bCell, const std::string &name,
                    int content, int optAddr = -1, elemLib *refElemLib = 0,
                    sramNameSpSite *refSramNameSp = 0)
      : _basicCell(bCell), _name(name), _content(content), _optAddr(optAddr),
        _refElemLib(refElemLib), _refSramNameSp(refSramNameSp) {}
};

class funcBase : public CilBase {
protected:
  std::string _quomodo;
  std::string _manner;

public:
  funcBase(const std::string &name, const std::string &quomodo = "",
           const std::string &manner = "")
      : CilBase(name), _quomodo(quomodo), _manner(manner) {}

  std::string getQuomodo() const { return _quomodo; }

  void transform(cfgFunc &func) {
    func._name = _name;
    func._quomodo = _quomodo;
    func._manner = _manner;
  }
};

class funcInCfgElemSite : public funcBase {
public:
  using sramsInFunc = cilContainer<sramInCfgElemSite>;

private:
  sramsInFunc _srams;
  bool _default;
  elemLib *_refElemLib;
  sramNameSpSite *_refSramNameSp;

public:
  funcInCfgElemSite(const std::string &name, const std::string &quodomo = "",
                    const std::string &manner = "", bool defaultOrNot = false,
                    elemLib *refElemLib = 0, sramNameSpSite *refSramNameSp = 0)
      : funcBase(name, quodomo, manner), _default(defaultOrNot),
        _refElemLib(refElemLib), _refSramNameSp(refSramNameSp) {}

  void addSram(sramInCfgElemSite *sram) { _srams.add(sram); }
  sramsInFunc &getSrams() { return _srams; }

  bool isDefault() const { return _default; }
  void getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement);
};

class cfgElemSite : public CilBase {
public:
  using functionsType = cilContainer<funcInCfgElemSite>::range_type;
  using const_functionsType = cilContainer<funcInCfgElemSite>::const_range_type;
  using functionIter = cilContainer<funcInCfgElemSite>::iterator;
  using const_functionIter = cilContainer<funcInCfgElemSite>::const_iterator;

private:
  cilContainer<funcInCfgElemSite> _functions;
  elemLib *_refElemLib;
  sramNameSpSite *_refSramNameSp;

public:
  cfgElemSite(const std::string &name, elemLib *refElemLib = 0,
              sramNameSpSite *refSramNameSp = 0)
      : CilBase(name), _refElemLib(refElemLib), _refSramNameSp(refSramNameSp) {}

  functionsType functions() { return _functions.range(); }
  const_functionsType functions() const { return _functions.range(); }

  funcInCfgElemSite *addFunc(funcInCfgElemSite *func) {
    return _functions.add(func);
  }
  funcInCfgElemSite *getFunc(const std::string &name) {
    return functions().find(name);
  }
  funcInCfgElemSite *getFunc(const cfgElem &cfgElement);

  void listDfotCfgs(vecCfgs &cfgs);
  void getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement);
};

class cfgElemsSite {
public:
  using cfgElemsType = cilContainer<cfgElemSite>::range_type;
  using const_cfgElemsType = cilContainer<cfgElemSite>::const_range_type;
  using cfgElemIter = cilContainer<cfgElemSite>::iterator;
  using const_cfeElemIter = cilContainer<cfgElemSite>::const_iterator;

private:
  cilContainer<cfgElemSite> _cfgElements;
  elemLib *_refElemLib;
  sramNameSpSite *_refSramNameSp;

public:
  cfgElemsSite(elemLib *refElemLib = 0, sramNameSpSite *refSramNameSp = 0)
      : _refElemLib(refElemLib), _refSramNameSp(refSramNameSp) {}

  void setRefElemLib(elemLib *ref) { _refElemLib = ref; }
  elemLib *getRefElemLib() const { return _refElemLib; }

  void setRefNameSp(sramNameSpSite *ref) { _refSramNameSp = ref; }
  sramNameSpSite *getRefNameSp() const { return _refSramNameSp; }

  cfgElemsType cfgElems() { return _cfgElements.range(); }
  const_cfgElemsType cfgElems() const { return _cfgElements.range(); }

  cfgElemSite *addCfgElem(cfgElemSite *cfgElem) {
    return _cfgElements.add(cfgElem);
  }
  cfgElemSite &getCfgElem(const std::string &name) {
    return *cfgElems().find(name);
  }

  void listDfotCfgs(vecCfgs &cfgs);
  void getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// cfgInfo in site
class cfgInfoSite {
private:
  sramNameSpSite _sramNameSp;
  cfgElemsSite _cfgElems;
  elemLib *_refElemLib;

public:
  explicit cfgInfoSite(elemLib *refElemLib = 0) : _refElemLib(refElemLib) {}

  void setRefElemLib(elemLib *ref) { _refElemLib = ref; }
  elemLib *getRefElemLib() const { return _refElemLib; }

  sramNameSpSite &getSramNameSp() { return _sramNameSp; }
  cfgElemsSite &getCfgElems() { return _cfgElems; }

  void listDfotCfgs(vecCfgs &cfgs);
  void getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement);
};

} // namespace cil_lib
} // namespace FDU

#endif