#ifndef ARCH_VALIDATION_H
#define ARCH_VALIDATION_H

#include "arch/archlib.hpp"
#include <iostream>

namespace FDU {
namespace RT {
using namespace ARCH;
using std::ostream;
using std::string;

class ArchValidationBase {
public:
  ArchValidationBase();
  virtual void validate() const = 0;
  ostream &println(ostream &os, const string &str) const;

protected:
  FPGADesign *_arch;
};

class ArchValidation : public ArchValidationBase {
public:
  virtual void validate() const;
};

class TilePortValidation : public ArchValidationBase {
public:
  virtual void validate() const;

private:
  void vldtInstance(const ArchInstance *inst, ostream &os) const;
  void vldtInstanceSide(const ArchInstance *inst, SideType side,
                        ostream &os) const;
  bool portsFit(const Port *lhs, const Port *rhs) const;
};
} // namespace RT
} // namespace FDU

#endif