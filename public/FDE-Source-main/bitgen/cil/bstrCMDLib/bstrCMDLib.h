#ifndef _BSTRCMDLIB_H_
#define _BSTRCMDLIB_H_

#include "container/Container.h"

namespace FDU {
namespace cil_lib {

class bstrCMD : public CilBase {
private:
  std::string _Cmd;
  std::string _Parameter;

public:
  explicit bstrCMD(std::string Cmd, std::string Parameter)
      : CilBase(""), _Cmd(Cmd), _Parameter(Parameter) {}
  std::string getCmd() { return _Cmd; }
  std::string getParameter() { return _Parameter; }
};

class bstrParameter : public CilBase {
private:
  int _Value;

public:
  explicit bstrParameter(std::string name, int value)
      : CilBase(name), _Value(value) {}
  int getParameterValue() { return _Value; }
};

class bstrCMDLib {
public:
  using bstrCMDsType = cilContainer<bstrCMD>::range_type;
  using const_bstrCMDsType = cilContainer<bstrCMD>::const_range_type;
  using bstrCMDsIter = cilContainer<bstrCMD>::iterator;
  using const_bstrCMDsIter = cilContainer<bstrCMD>::const_iterator;

  using bstrParametersType = cilContainer<bstrParameter>::range_type;
  using const_bstrParametersType =
      cilContainer<bstrParameter>::const_range_type;
  using bstrParametersIter = cilContainer<bstrParameter>::iterator;
  using const_bstrParametersIter = cilContainer<bstrParameter>::const_iterator;

private:
  cilContainer<bstrCMD> _bstrCMDs;
  cilContainer<bstrParameter> _bstrParameters;

public:
  bstrParametersType bstrParameters() { return _bstrParameters.range(); }
  const_bstrParametersType bstrParameters() const {
    return _bstrParameters.range();
  }
  bstrParameter *addbstrParameter(bstrParameter *bstrparameter) {
    return _bstrParameters.add(bstrparameter);
  }
  bstrParameter &getbstrParameter(std::string name) {
    return *bstrParameters().find(name);
  }

  bstrCMDsType bstrCMDs() { return _bstrCMDs.range(); }
  const_bstrCMDsType bstrCMDs() const { return _bstrCMDs.range(); }
  bstrCMD *addbstrCMD(bstrCMD *bstrcmd) { return _bstrCMDs.add(bstrcmd); }
};

} // namespace cil_lib
} // namespace FDU

#endif