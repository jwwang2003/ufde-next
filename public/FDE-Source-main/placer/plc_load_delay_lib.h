#ifndef PLC_LOADDELAYTABLE_H
#define PLC_LOADDELAYTABLE_H

#include "plc_device.h"
#include "xmlutils.h"
#include <boost/tokenizer.hpp>

namespace FDU {
namespace Place {
using namespace FDU::XML;
using namespace DEVICE;

class DelayTableHandler {
public:
  typedef boost::tokenizer<boost::char_separator<char>> Tokens;

  DelayTableHandler() : _has_table(false) {}

  void load_design(xml_node *node);
  void load_delay(xml_node *node);
  void load_table(DeviceInfo::DelayTable &table, const Point &scale,
                  const Tokens &tokens);

private:
  bool _has_table;
  DelayTableType _delay_table_type;
  Point _delay_table_scale;
};
} // namespace Place
} // namespace FDU
#endif