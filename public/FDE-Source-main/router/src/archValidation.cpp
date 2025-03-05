#include "archValidation.h"
#include "utils.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <string>

#define PRINTLN(os, line) os << (line) << std::endl

namespace FDU {
namespace RT {
namespace VALIDATION_LOG_PATH {
const string LOG_FOLDER = "arch_validation_info\\";
const string tilePortInfoFile = "tile_port_info";
} // namespace VALIDATION_LOG_PATH

using boost::format;
using boost::lexical_cast;
using std::ofstream;
using std::ostream;
using std::string;
using namespace VALIDATION_LOG_PATH;

ArchValidationBase::ArchValidationBase() : _arch(FPGADesign::instance()) {}

void ArchValidation::validate() const {
  TilePortValidation tpv;
  tpv.validate();
}

void TilePortValidation::validate() const {
  // need to add: if the folder does not exist, just create it.
  ofstream ofs((LOG_FOLDER + tilePortInfoFile).c_str());
  for (ArchInstance *inst :
       static_cast<ArchCell *>(
           _arch->find_library("arch")->find_module(_arch->name()))
           ->instances()) {
    vldtInstance(inst, ofs);
  }
  ofs.close();
}

void TilePortValidation::vldtInstance(const ArchInstance *inst,
                                      ostream &os) const {
  const Point archScale = _arch->scale();
  if (inst->logic_pos().x < archScale.x - 1) {
    vldtInstanceSide(inst, BOTTOM, os);
  }
  if (inst->logic_pos().y < archScale.y - 1) {
    vldtInstanceSide(inst, RIGHT, os);
  }
}

void TilePortValidation::vldtInstanceSide(const ArchInstance *inst,
                                          SideType side, ostream &os) const {
  ASSERT(side == BOTTOM || side == RIGHT,
         "TilePortValidation: I just accept side of BOTTOM OR RIGHT.");
  const SideType oppositeSide = side == BOTTOM ? TOP : LEFT;

  const string sideStr = lexical_cast<string>(side);
  const string oppositeSideStr = lexical_cast<string>(oppositeSide);

  const Point pos = inst->logic_pos();
  const Point oppositePos =
      side == BOTTOM ? Point(pos.x + 1, pos.y) : Point(pos.x, pos.y + 1);

  const Port *pt = inst->down_module()->find_port(sideStr);
  const ArchInstance *oppositeInst = _arch->get_inst_by_pos(oppositePos);
  const Port *oppositePt =
      oppositeInst->down_module()->find_port(oppositeSideStr);

  if (pt == nullptr && oppositePt != nullptr) {
    PRINTLN(os, format("%s(%s) has no %s side port but its opposite tile "
                       "%s(%s) has %s port.") %
                    inst->name() % lexical_cast<string>(pos) % sideStr %
                    oppositeInst->name() % lexical_cast<string>(oppositePos) %
                    oppositeSideStr);
  }
  if (oppositePt == nullptr && pt != nullptr) {
    PRINTLN(os, format("%s(%s) has no %s side port but its opposite tile "
                       "%s(%s) has %s port.") %
                    oppositeInst->name() % lexical_cast<string>(oppositePos) %
                    oppositeSideStr % inst->name() % lexical_cast<string>(pos) %
                    sideStr);
  }
  if (pt && oppositePt && !portsFit(pt, oppositePt)) {
    PRINTLN(
        os,
        format(
            "%s(%s)'s %s side port AND %s(%s)'s %s side port do not match.") %
            inst->name() % lexical_cast<string>(pos) % sideStr %
            oppositeInst->name() % lexical_cast<string>(oppositePos) %
            oppositeSideStr);
  }
}

bool TilePortValidation::portsFit(const Port *lhs, const Port *rhs) const {
  return lhs->msb() == rhs->msb() && lhs->lsb() == rhs->lsb();
}
} // namespace RT
} // namespace FDU