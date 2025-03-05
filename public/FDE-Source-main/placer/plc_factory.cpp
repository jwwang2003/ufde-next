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
/*	���ܣ�����һ��net��tcost
 *	������dmax, crit_exp
 *	����ֵ��void
 *	˵��������ignored net���洢
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
/*	���ܣ��洢ÿ��net���ӵ�instance
 *	������void
 *	����ֵ��void
 *	˵��������ignored net���洢
 */
/************************************************************************/
void PLCNet::store_connected_insts() {
  if (_is_ignored)
    return;
  // ���ø���pins()�õ�net��pins
  for (Pin *pin : pins()) {
    // �õ�pin���ӵ�instance
    PLCInstance *p_inst = static_cast<PLCInstance *>(pin->owner());
    // Ѱ���Ƿ��Ѿ�����������
    ConnectedInsts::iterator inst_iter =
        find(_connected_insts.begin(), _connected_insts.end(), p_inst);
    // ���û�з������棬��ô�������
    if (inst_iter == _connected_insts.end())
      _connected_insts.push_back(p_inst);
  }
}

} // namespace Place
} // namespace FDU