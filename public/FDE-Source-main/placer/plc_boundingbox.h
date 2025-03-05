#ifndef PLCBOUNDINGBOX_H
#define PLCBOUNDINGBOX_H

#include "plc_utils.h"

namespace FDU {
namespace Place {

class PLCNet;
class DeviceInfo;

class BoundingBox {
public:
  BoundingBox(PLCNet *net)
      : _owner(net), _x_max(0), _x_min(0), _y_max(0), _y_min(0), _bb_cost(0),
        _pre_bb_cost(-1) {}

  double compute_bb_cost(double inv_of_chan_width);

  void reset_pre_bb_cost() { _pre_bb_cost = -1; }
  void set_pre_bb_cost(double cost) { _pre_bb_cost = cost; }
  double bb_cost() const { return _bb_cost; };
  double pre_bb_cost() const { return _pre_bb_cost; }

  void save_bounding_box() {
    _pre_x_max = _x_max;
    _pre_x_min = _x_min;
    _pre_y_max = _y_max;
    _pre_y_min = _y_min;
    _pre_x_max_edge = _x_max_edge;
    _pre_x_min_edge = _x_min_edge;
    _pre_y_max_edge = _y_max_edge;
    _pre_y_min_edge = _y_min_edge;
    _pre_bb_cost = _bb_cost;
  }

  void restore_bounding_box() {
    _x_max = _pre_x_max;
    _x_min = _pre_x_min;
    _y_max = _pre_y_max;
    _y_min = _pre_y_min;
    _x_max_edge = _pre_x_max_edge;
    _x_min_edge = _pre_x_min_edge;
    _y_max_edge = _pre_y_max_edge;
    _y_min_edge = _pre_y_min_edge;
    _bb_cost = _pre_bb_cost;
    _pre_bb_cost = -1;
  }

  // compute the bounding box of the owner net
  virtual void compute_bounding_box(const Point &device_scale);
  // incrementally update bounding box
  virtual void update_bounding_box(const Point &device_scale,
                                   const Point &pos_old, const Point &pos_new);

private:
  int _x_max, _x_min, _y_max, _y_min;
  int _x_max_edge, _x_min_edge, _y_max_edge, _y_min_edge;
  int _pre_x_max, _pre_x_min, _pre_y_max, _pre_y_min;
  int _pre_x_max_edge, _pre_x_min_edge, _pre_y_max_edge, _pre_y_min_edge;
  double _bb_cost, _pre_bb_cost;
  PLCNet *_owner;
};
} // namespace Place
} // namespace FDU

#endif