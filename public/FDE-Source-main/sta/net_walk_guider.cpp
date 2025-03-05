#include "net_walk_guider.hpp"
#include "sta_arg.hpp"

using namespace ARCH;
using namespace boost;
using namespace std;

namespace FDU {
namespace STA {

NWGFactory::pointer NWGFactory::_instance(new NWGFactory);

void NetWalkGuider::build() {
  Library *lib = FPGADesign::instance()->find_library("tile");
  ASSERT(lib, "tile library not found");
  for (Module *tile_cell : lib->modules()) {
    TilePtr ptile = NWGFactory::instance().make_tile(tile_cell->name());
    FDU_LOG(VERBOSE) << "create tile type: " << tile_cell->name();

    for (Port *port : tile_cell->ports())
      ASSERTS(port->is_vector()); // check port description method

    for (Pin *pin : tile_cell->pins())
      // create pins repository
      ptile->add_pin(pin->name(),
                     NWGFactory::instance().make_pin(
                         pin->index(), static_cast<ArchPort *>(pin->port())));

    for (Net *net : tile_cell->nets()) {
      bool connect_to_out = false;
      typedef vector<Pin *> vec_pin;
      vec_pin grm_pins, self_pins;

      //! 1. analysis the pins owner connected with net
      for (Pin *pin : net->pins()) {
        if (pin->is_mpin()) {
          connect_to_out = true;
          self_pins.push_back(pin);
        } else {
          ArchInstance *pin_owner = static_cast<ArchInstance *>(pin->owner());
          if (pin_owner->down_module()->type() == STAArg::instance().grm_name) {
            grm_pins.push_back(pin);
          }
        }
      }

      //! 2. handle pin connection relationships
      if (!connect_to_out)
        continue; // check net has connected to tile pin

      // create "grm_net => self pins" map
      for (const Pin *grm_pin : grm_pins) {
        if (grm_pin->down_pin()->is_connected()) {
          string grm_net_name = grm_pin->down_pin()->net()->name();
          for (const Pin *self_pin : self_pins) {
            TilePin *pin = ptile->find_pin(self_pin->name());
            ASSERT(pin, "cannot find pin: " + self_pin->path_name());
            ptile->add_grm_net_pin(grm_net_name, pin);
          }
        } else {
          FDU_LOG(WARN) << "dangling: " + grm_pin->down_pin()->path_name()
                        << endl;
        }
      }

      // create "self pin => self pins" map
      if (self_pins.size() >= 2) {
        for (vec_pin::const_iterator it_u = self_pins.begin();
             it_u != self_pins.end(); ++it_u)
          for (vec_pin::const_iterator it_d = it_u + 1; it_d != self_pins.end();
               ++it_d) {
            TilePin *pin_u = ptile->find_pin((*it_u)->name());
            TilePin *pin_d = ptile->find_pin((*it_d)->name());
            ASSERTS(pin_u && pin_d);
            ptile->add_self_pin_pin((*it_u)->name(), pin_d);
            ptile->add_self_pin_pin((*it_d)->name(), pin_u);
          }
      }
    }

    // end pin map creation
    tile_repo_.insert(make_pair(ptile->name(), ptile));
  }
}

TilePin *NWGFactory::make_pin(int index, ArchPort *owner) {
  return new TilePin(index, owner);
}

TilePtr NWGFactory::make_tile(const std::string &name) {
  return TilePtr(new Tile(name));
}

} // namespace STA
} // namespace FDU
