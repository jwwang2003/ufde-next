#ifndef PLCCONSTINFER_H
#define PLCCONSTINFER_H

#include <map>

namespace COS {
class Instance;
class Net;
class Module;
class TDesign;
} // namespace COS

namespace FDU {
namespace Place {

using namespace COS;

class ConstantsGener {
public:
  ConstantsGener() : _const_slice_cell(nullptr), _const_slice_inst(nullptr) {}

  void generate_constants(TDesign *design);

protected:
  std::pair<Net *, Net *> find_constants_nets(TDesign *design);

  void generate_dummy_cell(TDesign *design, Net *gnd_net, Net *vcc_net);
  void generate_gnd_part(TDesign *design);
  void generate_vcc_part(TDesign *design);
  void generate_dummy_inst(TDesign *design, Net *gnd_net, Net *vcc_net);

private:
  Module *_const_slice_cell;
  Instance *_const_slice_inst;
};

} // namespace Place
} // namespace FDU

#endif