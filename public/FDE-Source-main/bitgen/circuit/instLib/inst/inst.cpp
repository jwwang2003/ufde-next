#include "circuit/instLib/inst/inst.h"
#include "log.h"

#include <boost/regex.hpp>

namespace BitGen {
namespace circuit {
using namespace boost;

void Inst::adaptCfgTo(vecCfgs &cfgs, const cfgInst &cfg) {
  cfgElem tCfg;
  tCfg._tileName = _position._tile;
  tCfg._siteName = _position._site;
  tCfg._cfgElemName = cfg.getName();
  cfg.getFunction().toFunction(tCfg._cfgElemFunc);
  cfgs.push_back(tCfg);
}

void Inst::listInstCfgs(vecCfgs &cfgs) {
  for (const cfgInst *cfg : _cfgs.configs())
    adaptCfgTo(cfgs, *cfg);
}

//////////////////////////////////////////////////////////////////////////
// for XML

void Inst::constructFromXML() {
  COS::Instance *inst = static_cast<COS::Instance *>(_nlBase);
  Property<Point> &postion = create_property<Point>(COS::INSTANCE, "position");
  Point pos = inst->property_value(postion);
  ArchInstance *tile = _archLibrary->get_inst_by_pos(pos);

  _name = inst->name();
  _instType = inst->down_module()->type();

  string site = "";
  for (const ArchInstance *archInst : tile->down_module()->instances()) {
    if (archInst->down_module()->type() == _instType &&
        archInst->z_pos() == pos.z) {
      site = archInst->name();
      break;
    }
  }
  ASSERT(!site.empty(),
         "Inst: can't find reference instance in tile ... " + _name);
  _position.constructFromXML("placed", tile->name(), site);

  for (const PropertyBase *prop : inst->properties()) {
    if (prop->name() != "h_set" && prop->name() != "position" &&
        prop->name() != "rloc" && prop->name() != "set_type")
      _cfgs.addCfgXML(prop->name(), prop->string_value(inst));
  }

  // for new netlist
  for (const Config *cfg : inst->configs())
    _cfgs.addCfgXML(cfg->name(), inst->property_value(*cfg));
}

} // namespace circuit
} // namespace BitGen