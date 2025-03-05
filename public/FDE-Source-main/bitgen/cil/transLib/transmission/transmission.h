#ifndef _TRANSMISSION_H_
#define _TRANSMISSION_H_

#include "cil/cilBase.h"
#include "cil/siteLib/siteLib.h"

namespace FDU {
namespace cil_lib {

class Trans : public CilBase {
private:
  std::string _refSiteName;
  siteLib *_refSiteLib;

public:
  Trans(const std::string &transName, const std::string &refSiteName,
        siteLib *refSiteLib = 0)
      : CilBase(transName), _refSiteName(refSiteName), _refSiteLib(refSiteLib) {
  }

  std::string getRefSiteName() const { return _refSiteName; }
};

} // namespace cil_lib
} // namespace FDU

#endif