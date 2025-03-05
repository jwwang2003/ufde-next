#ifndef FDP1000K_ARCHLIB_H
#define FDP1000K_ARCHLIB_H

#include "netlist.hpp"

namespace ARCH {

using namespace COS;

enum SideType { SIDE_IGNORE = -1, LEFT, BOTTOM, RIGHT, TOP, GLOBAL };

class ArchCell;

class ArchFactory : public CosFactory {
public:
  Module *make_module(const string &name, const string &type, Library *owner);
  Instance *make_instance(const string &name, Module *down_module,
                          Module *owner);
  Port *make_port(const string &name, int msb, int lsb, DirType dir,
                  PortType type, bool is_vec, Module *owner, int pin_from);
  COS::XML::NetlistHandler *make_xml_read_handler();
};

struct PadInfo {
  string name;
  Point position;

  PadInfo(const string &_n, Point _p) : name(_n), position(_p) {}
};

class ArchInstance : public Instance {
public:
  ArchInstance(const string &name, Module *down_module, Module *owner);

  int z_pos() const { return _zpos; }
  const Point &phy_pos() const { return _phy_pos; }
  const Point &logic_pos() const { return _logic_pos; }
  const Point &bit_pos() const { return _bit_pos; }

  void set_zpos(int zpos) { _zpos = zpos; }
  void set_phy_pos(const Point &pos) { _phy_pos = pos; }
  void set_logic_pos(const Point &pos) { _logic_pos = pos; }
  void set_bit_pos(const Point &pos) { _bit_pos = pos; }

  ArchCell *down_module() const;
  ArchCell *module() const;

private:
  int _zpos;
  Point _phy_pos;
  Point _logic_pos;
  Point _bit_pos;
};

class ArchPort : public Port {
public:
  ArchPort(const string &name, int msb, int lsb, DirType dir, PortType type,
           bool is_vec, Module *owner, int pin_from);

  SideType side() const { return _side; }
  void set_side(SideType side) { _side = side; }

  string pin_name_format() const { return "%s%d"; }

private:
  SideType _side;
};

class ArchPath {
public:
  struct Timing {
    double _R;
    double _Cin;
    double _Cout;
    double _intrinsic_delay;

    Timing(double R = 0., double Cin = 0., double Cout = 0., double delay = 0.)
        : _R(R), _Cin(Cin), _Cout(Cout), _intrinsic_delay(delay) {}
  };

  typedef ArchCell owner_type;

  ArchPath(owner_type &owner, ArchPort *from_port, ArchPort *to_port, int type)
      : _owner(&owner), _from_port(from_port), _to_port(to_port), _type(type) {}

  owner_type *owner() const { return _owner; }
  ArchPort *from_port() const { return _from_port; }
  ArchPort *to_port() const { return _to_port; }
  int type() const { return _type; }

  double R() const { return _timing._R; }
  double Cin() const { return _timing._Cin; }
  double Cout() const { return _timing._Cout; }
  double delay() const { return _timing._intrinsic_delay; }
  void set_timing(double R, double Cin, double Cout, double delay) {
    _timing = Timing(R, Cin, Cout, delay);
  }

private:
  owner_type *_owner;
  ArchPort *_from_port;
  ArchPort *_to_port;
  int _type;
  Timing _timing;
};

class ArchCell : public Module {
public:
  ArchCell(const string &name, const string &type, Library *owner);

  //  Instances
  typedef PtrList<Instance>::typed<ArchInstance>::range instances_type;
  typedef PtrList<Instance>::typed<ArchInstance>::const_range
      const_instances_type;
  typedef PtrList<Instance>::typed<ArchInstance>::iterator inst_iter;
  typedef PtrList<Instance>::typed<ArchInstance>::const_iterator
      const_inst_iter;

  instances_type instances() {
    return static_cast<instances_type>(Module::instances());
  }
  const_instances_type instances() const {
    return static_cast<const_instances_type>(Module::instances());
  }

  // Port
  typedef PtrVector<Port>::typed<ArchPort>::range ports_type;
  typedef PtrVector<Port>::typed<ArchPort>::const_range const_ports_type;
  typedef PtrVector<Port>::typed<ArchPort>::iterator port_iter;
  typedef PtrVector<Port>::typed<ArchPort>::const_iterator const_port_iter;

  ports_type ports() { return static_cast<ports_type>(Module::ports()); }
  const_ports_type ports() const {
    return static_cast<const_ports_type>(Module::ports());
  }

  // Path
  typedef PtrVector<ArchPath>::const_range_type const_path_type;
  typedef PtrVector<ArchPath>::range_type path_type;
  typedef PtrVector<ArchPath>::const_iterator const_path_iter;
  typedef PtrVector<ArchPath>::iterator path_iter;

  ArchPath *create_path(const string &, const string &, int);

  path_type paths() { return _paths.range(); }
  const_path_type paths() const { return _paths.range(); }

private:
  PtrVector<ArchPath> _paths;
};

struct WireTiming {
  double _unit_R;
  double _unit_C;

  WireTiming(double unit_R = 0., double unit_C = 0.)
      : _unit_R(unit_R), _unit_C(unit_C) {}
};

class FPGADesign : public Design {
public:
  const Point &scale() const { return _scale; }
  void set_scale(const Point &scale) { _scale = scale; }

  void set_package(const string &package) { _package = package; }
  string get_package() const { return _package; }

  void set_wire_timing(double R, double C) { _wire_timing = WireTiming(R, C); }
  double wire_unit_resistor() const { return _wire_timing._unit_R; }
  double wire_unit_capacity() const { return _wire_timing._unit_C; }

  void add_pad(const string &n, const Point &p) {
    _pads.push_back(PadInfo(n, p));
  }
  Point find_pad_by_name(const string &n);
  string find_pad_by_position(const Point &pos) const;
  bool is_io_bound(const Point &p);
  bool is_io_bound(const string &n);

  void set_LUT_inputs(int num) { _LUT_inputs = num; }
  auto get_LUT_inputs() const { return _LUT_inputs; }

  void set_slice_num(int num) { _slice_per_tile = num; }
  auto get_slice_num() const { return _slice_per_tile; }
  void add_carry_chain(vector<int> chain) { _carry_chain.push_back(chain); }
  vector<vector<int>> get_carry_chain() { return _carry_chain; }
  //		void	set_carry_num(int carry_num)		  {_carry_num =
  // carry_num;}
  auto get_carry_num() { return _carry_chain.size(); }

  void add_bram_col(int col) { _bram_col.push_back(col); }
  bool is_bram(int col);

  ArchInstance *get_inst_by_pos(const Point &p);

  static FPGADesign *loadArchLib(const string &file,
                                 CosFactory *factory = new ArchFactory());
  static FPGADesign *instance() { return _instance.get(); }
  static void release() { _instance.reset(); }

private:
  string _package;
  Point _scale;
  vector<PadInfo> _pads;
  WireTiming _wire_timing;
  int _slice_per_tile;
  vector<vector<int>> _carry_chain;
  //		int				   _carry_num;
  vector<int> _bram_col;

  int _LUT_inputs;

  static std::unique_ptr<FPGADesign> _instance;
};

inline ArchCell *ArchInstance::down_module() const {
  return static_cast<ArchCell *>(Instance::down_module());
}
inline ArchCell *ArchInstance::module() const {
  return static_cast<ArchCell *>(Instance::module());
}

std::istream &operator>>(std::istream &s, SideType &type);
std::ostream &operator<<(std::ostream &s, SideType type);
} // namespace ARCH
#endif
