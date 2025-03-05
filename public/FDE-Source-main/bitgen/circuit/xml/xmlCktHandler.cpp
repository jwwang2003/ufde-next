#include "circuit/xml/xmlCktHandler.h"
#include "io/fileio.hpp"
#include "main/arguments/Args.h"
#include "rtnetlist.hpp"
#include "zfstream.h"

namespace BitGen {
namespace circuit {
using namespace boost;
using namespace COS;

void xmlCktHandler::parse(const std::string &file) {
  // load xml netlist
  CosFactory::pointer old_factory = CosFactory::set_factory(new COSRTFactory());
  create_property<Point>(COS::INSTANCE, "position");
  _design.load("xml", file);

  CosFactory::set_factory(old_factory);

  // get top_cell
  Module *topCell = _design.top_module();

  // design
  _curCkt->getDesign()->constructFromXML(_design.name());

  // instances
  for (COS::Instance *inst : topCell->instances())
    _curCkt->getInsts()->addInst(new Inst(_curCkt->getArchLibrary(), inst));

  // nets
  for (COS::Net *net : topCell->nets())
    _curCkt->getNets()->addNet(
        new circuit::Net(_curCkt->getArchLibrary(), net, _curCkt->getInsts()));
}

} // namespace circuit
} // namespace BitGen