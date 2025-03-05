#ifndef PLCLUT6INFER_H
#define PLCLUT6INFER_H

#include <map>
#include <vector>

namespace COS {
class TDesign;
}

namespace FDU {
namespace Place {

using namespace COS;

/************************************************************************/
/* 处理网表中的LUT6s                                                    */
/************************************************************************/
class PLCInstance;
class LUT6Inference {
public:
  typedef std::vector<std::pair<PLCInstance *, PLCInstance *>> LUT6s;
  // 找到design网表中的LUT6并且存储在lut6s里面
  void inference(TDesign *design, LUT6s &lut6s);
};

} // namespace Place
} // namespace FDU

#endif