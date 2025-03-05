#ifndef _CILLIB_H_
#define _CILLIB_H_

#include "arch/archlib.hpp"
#include "cil/bramLib/bramLib.h"
#include "cil/bstrCMDLib/bstrCMDLib.h"
#include "cil/clusterLib/clusterLib.h"
#include "cil/elementLib/elemLib.h"
#include "cil/majorLib/majorLib.h"
#include "cil/siteLib/siteLib.h"
#include "cil/tileLib/tileLib.h"
#include "cil/transLib/transLib.h"

namespace FDU {
namespace cil_lib {
using namespace ARCH;

class cilLibrary {
private:
  string _chipName;
  FPGADesign *_archLibrary;
  elemLib _elemLibrary;
  siteLib _siteLibrary;
  clstLib _clstLibrary;
  trnsLib _trnsLibrary;
  tileLib _tileLibrary;
  bramLib _bramLibrary;
  majorLib _majorLibrary;
  bstrCMDLib _bstrCMDLibrary;

public:
  cilLibrary()
      : _siteLibrary(&_elemLibrary), _clstLibrary(&_siteLibrary),
        _trnsLibrary(&_siteLibrary),
        _tileLibrary(&_clstLibrary, &_trnsLibrary) {}

  static cilLibrary *loadCilLib(const std::string &file);
  void setArchLib(FPGADesign *archlib);

  void setChipName(const string &name) { _chipName = name; }
  string getChipName() const { return _chipName; }

  elemLib *getElemLib() { return &_elemLibrary; }
  siteLib *getSiteLib() { return &_siteLibrary; }
  clstLib *getClstLib() { return &_clstLibrary; }
  trnsLib *getTrnsLib() { return &_trnsLibrary; }
  tileLib *getTileLib() { return &_tileLibrary; }
  bramLib *getBramLib() { return &_bramLibrary; }
  majorLib *getMajorLib() { return &_majorLibrary; }
  FPGADesign *getArchLib() const { return _archLibrary; }
  bstrCMDLib *getbstrCMDLib() { return &_bstrCMDLibrary; }
};

} // namespace cil_lib
} // namespace FDU

#endif