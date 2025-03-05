#ifndef _CFGININST_H_
#define _CFGININST_H_

#include "container/Container.h"
#include "utils/cfgInTile.h"

namespace BitGen {
namespace circuit {
struct funcInCfgInst {
  std::string _funcName;
  std::string _workMode;

  funcInCfgInst(const std::string &funcName = "",
                const std::string &workMode = "")
      : _funcName(funcName), _workMode(workMode) {}

  void toFunction(cfgFunc &func);
};

inline void funcInCfgInst::toFunction(cfgFunc &func) {
  func._name = _funcName;
  func._quomodo = _workMode;
  func._manner = "";
}

class cfgInst {
private:
  std::string _instName;
  funcInCfgInst _function;
  bool _used;
  string _name;

public:
  // for XML
  std::string getName() const { return _name; }
  std::string name() const { return _name; }
  funcInCfgInst getFunction() const { return _function; }
  void constructFromXML();
  bool isUsed() const { return _used; }
  cfgInst(const std::string &name, const std::string &funcName)
      : _name(name), _function(funcName) {
    _name = name;
    constructFromXML();
  }
};

class contCfgsInst {
public:
  using cfgsInstType = cktContainer<cfgInst>::range_type;
  using const_cfgsInstType = cktContainer<cfgInst>::const_range_type;
  using cfgInstIter = cktContainer<cfgInst>::iterator;
  using const_cfgInstIter = cktContainer<cfgInst>::const_iterator;

private:
  cktContainer<cfgInst> _cfgs;

public:
  cfgsInstType configs() { return _cfgs.range(); }
  const_cfgsInstType configs() const { return _cfgs.range(); }
  void clear() { _cfgs.clear(); }
  void addCfgXML(const std::string &name, const std::string &funcName);
  cfgInst &getCfg(const std::string &cfgName) {
    return *configs().find(cfgName);
  }
};
} // namespace circuit
} // namespace BitGen

#endif