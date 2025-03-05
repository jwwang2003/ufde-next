#include "cil/cilLib.h"
#include "PropertyName.h"
#include "log.h"

namespace FDU {
namespace cil_lib {
using namespace COS;

void cilLibrary::setArchLib(FPGADesign *archlib) {
  _archLibrary = archlib;
  Library *primitiveLib = _archLibrary->find_library(ARCHNAME::PRIM);
  if (!primitiveLib) {
    primitiveLib = _archLibrary->find_library(ARCHNAME::BLOCK);
  }
  ASSERT(primitiveLib, "site: can not find primitive(block) library...");
  for (Site *site : _siteLibrary.sites()) {
    ArchCell *siteArch =
        static_cast<ArchCell *>(primitiveLib->find_module(site->getName()));
    ASSERTD(siteArch, "site: can't find reference cell in arch lib ... " +
                          site->getName());
    site->setRefSiteArch(siteArch);
  }
}

} // namespace cil_lib
} // namespace FDU