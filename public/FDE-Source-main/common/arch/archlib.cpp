#include "archlib.hpp"

namespace ARCH {

///////////////////////////////////////////////////////////////////////////////////////
//  streaming io for lexical_cast
static const char *sidetab[] = {"left", "bottom", "right", "top", "global", ""};
static EnumStringMap<SideType> sidemap(sidetab, SIDE_IGNORE);
std::istream &operator>>(std::istream &s, SideType &type) {
  type = sidemap.readEnum(s);
  return s;
}
std::ostream &operator<<(std::ostream &s, SideType type) {
  return sidemap.writeEnum(s, type);
}

//////////////////////////////////////////////////////////////////////////
// ArchFactory
Module *ArchFactory::make_module(const string &name, const string &type,
                                 Library *owner) {
  return new ArchCell(name, type, owner);
}
Instance *ArchFactory::make_instance(const string &name, Module *down_module,
                                     Module *owner) {
  return new ArchInstance(name, down_module, owner);
}
Port *ArchFactory::make_port(const string &name, int msb, int lsb, DirType dir,
                             PortType type, bool is_vec, Module *owner,
                             int pin_from) {
  return new ArchPort(name, msb, lsb, dir, type, is_vec, owner, pin_from);
}

//////////////////////////////////////////////////////////////////////////
// Constructor
ArchInstance::ArchInstance(const string &name, Module *down_module,
                           Module *owner)
    : Instance(name, down_module, owner), _zpos(-1) {}
ArchPort::ArchPort(const string &name, int msb, int lsb, DirType dir,
                   PortType type, bool is_vec, Module *owner, int pin_from)
    : Port(name, msb, lsb, dir, type, is_vec, owner, pin_from),
      _side(SIDE_IGNORE) {}
ArchCell::ArchCell(const string &name, const string &type, Library *owner)
    : Module(name, type, owner) {}

//////////////////////////////////////////////////////////////////////////
// ArchCell
ArchPath *ArchCell::create_path(const string &n_from, const string &n_to,
                                int type) {
  Port *from_port = ports().find(n_from);
  Port *to_port = ports().find(n_to);

  ASSERT(from_port && to_port, "port not found");
  return _paths.add(new ArchPath(*this, static_cast<ArchPort *>(from_port),
                                 static_cast<ArchPort *>(to_port), type));
}

//////////////////////////////////////////////////////////////////////////
// FPGADesign
std::unique_ptr<FPGADesign> FPGADesign::_instance; // the singleton instance

bool FPGADesign::is_bram(int col) {
  return std::find(_bram_col.begin(), _bram_col.end(), col) != _bram_col.end();
}

bool FPGADesign::is_io_bound(const Point &p) {
  return find_if(_pads.begin(), _pads.end(), [&p](const PadInfo &pad) {
           return pad.position == p;
         }) != _pads.end();
}

bool FPGADesign::is_io_bound(const string &n) {
  return find_if(_pads.begin(), _pads.end(), [&n](const PadInfo &pad) {
           return pad.name == n;
         }) != _pads.end();
}

Point FPGADesign::find_pad_by_name(const string &n) {
  auto it = find_if(_pads.begin(), _pads.end(),
                    [&n](const PadInfo &pad) { return pad.name == n; });
  return it == _pads.end() ? Point() : it->position;
}

string FPGADesign::find_pad_by_position(const Point &p) const {
  auto it = find_if(_pads.begin(), _pads.end(),
                    [&p](const PadInfo &pad) { return pad.position == p; });
  return it == _pads.end() ? string() : it->name;
}

ArchInstance *FPGADesign::get_inst_by_pos(const Point &p) {
  for (ArchInstance *inst : static_cast<ArchCell *>(top_module())->instances())
    if (inst->logic_pos().equal2d(p))
      return inst;
  return nullptr;
}
} // namespace ARCH