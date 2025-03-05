#ifndef NET_WALK_GUIDER
#define NET_WALK_GUIDER

#include "arch/archlib.hpp"
#include <boost/format.hpp>

using namespace ARCH;

namespace FDU {
namespace STA {

struct TilePin;
class Tile;

struct TilePin {

  typedef ARCH::SideType Side;

  TilePin(int index_, ArchPort *owner_) : index(index_), owner(owner_) {}
  virtual ~TilePin() {}

  std::string name() const {
    return (boost::format(owner->pin_name_format()) % owner->name() % index)
        .str();
  }

  std::string reverse_name() const {
    Side op_dir = static_cast<Side>((owner->side() + 2) % 4);
    return (boost::format(owner->pin_name_format()) %
            boost::lexical_cast<std::string>(op_dir) % index)
        .str();
  }

  Side dir() const { return owner->side(); }

  Point offset_point() const;

  int index;
  ArchPort *owner;
};

class Tile {

public:
  Tile(const std::string &name) : name_(name) {}
  virtual ~Tile() {
    for (pin_map::iterator it = pin_repo_.begin(); it != pin_repo_.end(); ++it)
      delete it->second;
  }

  typedef std::vector<TilePin *> Pins;
  typedef std::map<std::string, TilePin *> pin_map;
  typedef std::multimap<std::string, TilePin *> str_pins_map;

  void add_pin(const std::string &name, TilePin *pin) {
    pin_repo_.insert(std::make_pair(name, pin));
  }
  void add_grm_net_pin(const std::string &name, TilePin *pin) {
    grm_net_pins_.insert(std::make_pair(name, pin));
  }
  void add_self_pin_pin(const std::string &name, TilePin *pin) {
    self_pin_pins_.insert(std::make_pair(name, pin));
  }

  TilePin *find_pin(const std::string &pin_name) const;
  Pins find_grm_net_pins(const std::string &net_name) const;
  Pins find_self_pin_pins(const std::string &pin_name) const;

  const string &grm_type() const { return grm_type_; }
  const string &name() const { return name_; }

private:
  std::string name_;
  std::string grm_type_;

  pin_map pin_repo_;
  str_pins_map grm_net_pins_;
  str_pins_map self_pin_pins_;
};

typedef std::shared_ptr<Tile> TilePtr;

class NetWalkGuider {

public:
  // NetWalkGuider ();
  // virtual ~NetWalkGuider ();

  typedef std::map<std::string, TilePtr> tile_repo;
  typedef Tile::Pins Pins;

  Pins find_grm_net_pins(const std::string &tile_type,
                         const std::string &grm_net) const;
  Pins find_tile_pin_pins(const std::string &tile_type,
                          const std::string &pin_name) const;

  void build();

private:
  tile_repo tile_repo_;
};

class NWGFactory {
public:
  typedef std::unique_ptr<NWGFactory> pointer;

  virtual TilePin *make_pin(int index, ArchPort *owner);
  virtual TilePtr make_tile(const std::string &name);

  static NWGFactory &instance() { return *_instance.get(); }
  static pointer set_factory(pointer f) {
    pointer p = std::move(_instance);
    _instance = std::move(f);
    return p;
  }
  static pointer set_factory(NWGFactory *f) { return set_factory(pointer(f)); }

  virtual ~NWGFactory() {}

private:
  static pointer _instance;
};

/////////////////////////////////////////////////////////////////////
// TilePin

inline Point TilePin::offset_point() const {
  Point offset(0, 0);
  switch (dir()) {
  case LEFT:
    offset.y = -1;
    break;
  case RIGHT:
    offset.y = 1;
    break;
  case TOP:
    offset.x = -1;
    break;
  case BOTTOM:
    offset.x = 1;
    break;
  default:
    ASSERT(0, "unknown dir type");
    break;
  }
  return offset;
}

/////////////////////////////////////////////////////////////////////
// Tile

inline TilePin *Tile::find_pin(const std::string &pin_name) const {
  pin_map::const_iterator it = pin_repo_.find(pin_name);
  return it != pin_repo_.end() ? it->second : 0;
}

inline Tile::Pins Tile::find_grm_net_pins(const std::string &net_name) const {
  Pins pins;
  for (str_pins_map::const_iterator it = grm_net_pins_.lower_bound(net_name);
       it != grm_net_pins_.upper_bound(net_name); ++it)
    pins.push_back(it->second);
  return pins;
}

inline Tile::Pins Tile::find_self_pin_pins(const std::string &pin_name) const {
  Pins pins;
  for (str_pins_map::const_iterator it = self_pin_pins_.lower_bound(pin_name);
       it != self_pin_pins_.upper_bound(pin_name); ++it)
    pins.push_back(it->second);
  return pins;
}

////////////////////////////////////////////////////////////////////
// NetWalkGuider

inline NetWalkGuider::Pins
NetWalkGuider::find_grm_net_pins(const std::string &tile_type,
                                 const std::string &grm_net) const {
  tile_repo::const_iterator it = tile_repo_.find(tile_type);
  ASSERT(it != tile_repo_.end(), "cannot find tile: " + tile_type);
  return it->second->find_grm_net_pins(grm_net);
}

inline NetWalkGuider::Pins
NetWalkGuider::find_tile_pin_pins(const std::string &tile_type,
                                  const std::string &pin_name) const {
  tile_repo::const_iterator it = tile_repo_.find(tile_type);
  ASSERT(it != tile_repo_.end(), "cannot find tile: " + tile_type);
  return it->second->find_self_pin_pins(pin_name);
}

} // namespace STA
} // namespace FDU

#endif
