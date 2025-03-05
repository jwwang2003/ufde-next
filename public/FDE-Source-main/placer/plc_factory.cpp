#include "plc_factory.h"
#include "plc_utils.h"
#include "tengine/tengine.hpp"

namespace FDU {
namespace Place {

using namespace std;
using namespace COS;

//////////////////////////////////////////////////////////////////////////
// PLCNet
/************************************************************************/
/*	功能：计算一个net的tcost
 *	参数：dmax, crit_exp
 *	返回值：void
 *	说明：对于ignored net不存储
 */
/************************************************************************/
double PLCNet::compute_tcost(double dmax, double crit_exp) {
  double total_tcost = 0.;
  double crit, tcost;
  Property<double> &slacks = create_temp_property<double>(COS::PIN, PIN::SLACK);
  Property<COS::TData> &delays =
      create_temp_property<COS::TData>(COS::PIN, PIN::DELAY);

  Property<double> &crits = create_temp_property<double>(COS::PIN, PIN::CRIT);
  Property<double> &tcosts = create_temp_property<double>(COS::PIN, PIN::TCOST);

  for (Pin *pin : sink_pins()) {
    crit = max(1.0 - pin->property_value(slacks) / dmax, 0.);
    crit = pow(crit, crit_exp);
    tcost = crit * (pin->property_value(delays)._rising);

    total_tcost += tcost;

    pin->set_property(crits, crit);
    pin->set_property(crits, tcost);
  }

  return _total_tcost = total_tcost;
}
/************************************************************************/
/*	功能：存储每个net连接的instance
 *	参数：void
 *	返回值：void
 *	说明：对于ignored net不存储
 */
/************************************************************************/
void PLCNet::store_connected_insts() {
  if (_is_ignored)
    return;
  // 调用父类pins()得到net的pins
  for (Pin *pin : pins()) {
    // 得到pin连接的instance
    PLCInstance *p_inst = static_cast<PLCInstance *>(pin->owner());
    // 寻找是否已经放在里面了
    ConnectedInsts::iterator inst_iter =
        find(_connected_insts.begin(), _connected_insts.end(), p_inst);
    // 如果没有放在里面，那么将其放入
    if (inst_iter == _connected_insts.end())
      _connected_insts.push_back(p_inst);
  }
}

} // namespace Place
} // namespace FDU