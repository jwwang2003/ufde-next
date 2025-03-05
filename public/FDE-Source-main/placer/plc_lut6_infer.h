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
/* ���������е�LUT6s                                                    */
/************************************************************************/
class PLCInstance;
class LUT6Inference {
public:
  typedef std::vector<std::pair<PLCInstance *, PLCInstance *>> LUT6s;
  // �ҵ�design�����е�LUT6���Ҵ洢��lut6s����
  void inference(TDesign *design, LUT6s &lut6s);
};

} // namespace Place
} // namespace FDU

#endif