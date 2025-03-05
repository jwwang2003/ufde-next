#include "circuit.h"
#include "log.h"
#include "rtnetlist.hpp"

namespace BitGen {
namespace circuit {
Circuit::Circuit(const std::string &file, ARCH::FPGADesign *archLibrary)
    : _nets(&_insts) //, _archLibrary(archLibrary)
{
  // load xml netlist
  CosFactory::pointer old_factory = CosFactory::set_factory(new COSRTFactory());
  create_property<Point>(COS::INSTANCE, "position");
  _design.load("xml", file);
  CosFactory::set_factory(old_factory);

  // get top_cell
  Module *topCell = _design.top_module();

  // instances
  for (COS::Instance *inst : topCell->instances())
    _insts.addInst(new Inst(archLibrary, inst));

  // nets
  for (COS::Net *net : topCell->nets())
    _nets.addNet(new circuit::Net(archLibrary, net, &_insts));
}

} // namespace circuit
} // namespace BitGen