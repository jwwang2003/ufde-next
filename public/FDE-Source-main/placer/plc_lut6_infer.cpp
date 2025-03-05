#include "plc_lut6_infer.h"
#include "plc_factory.h"
#include "plc_utils.h"

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

namespace FDU {
namespace Place {

using namespace std;
using namespace boost;

/************************************************************************/
/*	功能：找到design网表中的LUT6并且存储在lut6s里面
 *	参数：design: 设计网表； lut6s: vector<pair>,存放找到的LUT6
 *	返回值：void
 *	说明：
 */
/************************************************************************/
void LUT6Inference::inference(TDesign *design, LUT6s &lut6s) {
  Property<string> &hsets =
      create_temp_property<string>(COS::INSTANCE, INSTANCE::HSET);
  Property<string> &SET_TYPE =
      create_property<string>(COS::INSTANCE, INSTANCE::SET_TYPE);
  for (PLCNet *net : static_cast<PLCModule *>(design->top_module())->nets()) {
    Net::pin_iter F5_it = find_if(
        net->pins(), [](const Pin *pin) { return pin->name() == DEVICE::F5; });
    Net::pin_iter F5IN_it = find_if(net->pins(), [](const Pin *pin) {
      return pin->name() == DEVICE::F5IN;
    });

    if (F5_it != net->pins().end() && F5IN_it != net->pins().end()) {
      // allow more pins besides F5 & F5IN
      //				ASSERT(net.num_pins() == 2,
      //					   place_error(CONSOLE::PLC_ERROR
      //% (net.name() + ": illegal LUT6 net.")));

      net->set_ignored();

      PLCInstance *src_slice = static_cast<PLCInstance *>(F5_it->owner());
      PLCInstance *sink_slice = static_cast<PLCInstance *>(F5IN_it->owner());

      src_slice->set_property(hsets, DEVICE::LUT_6);
      sink_slice->set_property(hsets, DEVICE::LUT_6);
      src_slice->set_property(SET_TYPE, DEVICE::LUT_6);
      sink_slice->set_property(SET_TYPE, DEVICE::LUT_6);

      lut6s.push_back(make_pair(src_slice, sink_slice));
    }
  }
}

} // namespace Place
} // namespace FDU