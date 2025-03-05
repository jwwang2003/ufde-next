#include "plc_boundingbox.h"
#include "plc_device.h"
#include "plc_factory.h"

namespace FDU {
namespace Place {

static const double cross_count[50] = { // [0..49]
    1.0,    1.0,    1.0,    1.0828, 1.1536, 1.2206, 1.2823, 1.3385, 1.3991,
    1.4493, 1.4974, 1.5455, 1.5937, 1.6418, 1.6899, 1.7304, 1.7709, 1.8114,
    1.8519, 1.8924, 1.9288, 1.9652, 2.0015, 2.0379, 2.0743, 2.1061, 2.1379,
    2.1698, 2.2016, 2.2334, 2.2646, 2.2958, 2.3271, 2.3583, 2.3895, 2.4187,
    2.4479, 2.4772, 2.5064, 2.5356, 2.5610, 2.5864, 2.6117, 2.6371, 2.6625,
    2.6887, 2.7148, 2.7410, 2.7671, 2.7933};

double BoundingBox::compute_bb_cost(double inv_of_chan_width) {
  static int num_terminals = _owner->num_pins();

  double cross;

  _bb_cost = _x_max - _x_min + 1;  // bb_x
  _bb_cost += _y_max - _y_min + 1; // bb_x + bb_y

  if (num_terminals > 50)
    cross = 2.7933 + 0.02616 * (num_terminals - 50);
  else
    cross = cross_count[num_terminals - 1];

  _bb_cost *= (cross * inv_of_chan_width);

  return _bb_cost;
}

void BoundingBox::compute_bounding_box(const Point &device_scale) {
  static int nx = device_scale.x;
  static int ny = device_scale.y;

  _x_min = nx + 1;
  _x_max = -1;
  _y_min = ny + 1;
  _y_max = -1;

  for (PLCInstance *inst : _owner->connected_insts()) {
    Point logic_pos = inst->curr_logic_pos();
    if (logic_pos.x == FLOORPLAN::INVALID_VALUE ||
        logic_pos.y == FLOORPLAN::INVALID_VALUE)
      continue;

    logic_pos = DeviceInfo::lookup_phy_pos(logic_pos);

    // x-direction
    if (logic_pos.x == _x_min) {
      _x_min_edge++;
    }
    if (logic_pos.x ==
        _x_max) { // Recall that xMin could equal xMax -- don't use else
      _x_max_edge++;
    }
    if (logic_pos.x < _x_min) {
      _x_min = logic_pos.x;
      _x_min_edge = 1;
    }
    if (logic_pos.x > _x_max) {
      _x_max = logic_pos.x;
      _x_max_edge = 1;
    }

    // y-direction
    if (logic_pos.y == _y_min) {
      _y_min_edge++;
    }
    if (logic_pos.y == _y_max) {
      _y_max_edge++;
    }
    if (logic_pos.y < _y_min) {
      _y_min = logic_pos.y;
      _y_min_edge = 1;
    }
    if (logic_pos.y > _y_max) {
      _y_max = logic_pos.y;
      _y_max_edge = 1;
    }
  }
}

void BoundingBox::update_bounding_box(const Point &device_scale,
                                      const Point &pos_old,
                                      const Point &pos_new) {
  Point phy_pos_old = DeviceInfo::lookup_phy_pos(pos_old);
  Point phy_pos_new = DeviceInfo::lookup_phy_pos(pos_new);

  // Check if the bounding box can be incrementally updated.

  if (phy_pos_new.x < phy_pos_old.x) { // Move to left.

    // Update the xMax fields for coordinates and number of edges first.

    if (phy_pos_old.x == _pre_x_max) { // Old position at xMax.
      if (_pre_x_max_edge == 1) {
        // force recompute
        compute_bounding_box(device_scale);
        return;
      } else
        _x_max_edge = _pre_x_max_edge - 1;
    }

    // Now do the xMin fields for coordinates and number of edges.

    if (phy_pos_new.x < _pre_x_min) { // Moved past xMin
      _x_min = phy_pos_new.x;
      _x_min_edge = 1;
    } else if (phy_pos_new.x == _pre_x_min)
      _x_min_edge = _pre_x_min_edge + 1;

  } // End of move to left case.

  else if (phy_pos_new.x > phy_pos_old.x) { // Move to right.

    // Update the xMin fields for coordinates and number of edges first.

    if (phy_pos_old.x == _pre_x_min) { // Old position at xMin.
      if (_pre_x_min_edge == 1) {
        // force recompute
        compute_bounding_box(device_scale);
        return;
      } else
        _x_min_edge = _pre_x_min_edge - 1;
    }

    // Now do the xMax fields for coordinates and number of edges.

    if (phy_pos_new.x > _pre_x_max) { // Moved past xMax.
      _x_max = phy_pos_new.x;
      _x_max_edge = 1;
    } else if (phy_pos_new.x == _pre_x_max) // Moved to xMax
      _x_max_edge = _pre_x_max_edge + 1;

  } // End of move to right case.

  // Now account for the y-direction motion.

  if (phy_pos_new.y < phy_pos_old.y) { // Move down.

    // Update the yMax fields for coordinates and number of edges first.

    if (phy_pos_old.y == _pre_y_max) { // Old position at yMax.
      if (_pre_y_max_edge == 1) {
        // force recompute
        compute_bounding_box(device_scale);
        return;
      } else
        _y_max_edge = _pre_y_max_edge - 1;
    }

    // Now do the yMin fields for coordinates and number of edges.

    if (phy_pos_new.y < _pre_y_min) { // Moved past yMin
      _y_min = phy_pos_new.y;
      _y_min_edge = 1;
    } else if (phy_pos_new.y == _pre_y_min) // Moved to yMin
      _y_min_edge = _pre_y_min_edge + 1;

  } // End of move down case.

  else if (phy_pos_new.y > phy_pos_old.y) { // Moved up.

    // Update the yMin fields for coordinates and number of edges first.

    if (phy_pos_old.y == _pre_y_min) { // Old position at yMin.
      if (_pre_y_min_edge == 1) {
        // force recompute
        compute_bounding_box(device_scale);
        return;
      } else
        _y_min_edge = _pre_y_min_edge - 1;
    }

    // Now do the yMax fields for coordinates and number of edges.

    if (phy_pos_new.y > _pre_y_max) { // Moved past yMax.
      _y_max = phy_pos_new.y;
      _y_max_edge = 1;
    } else if (phy_pos_new.y == _pre_y_max) // Moved to yMax
      _y_max_edge = _pre_y_max_edge + 1;

  } // End of move up case.

  // add for carry chain & lut6
  // because there are nets connect several elements with a carry chain or lut6
  if (_pre_x_min == _pre_x_max || _pre_y_min == _pre_y_max)
    compute_bounding_box(device_scale);
}

} // namespace Place
} // namespace FDU
