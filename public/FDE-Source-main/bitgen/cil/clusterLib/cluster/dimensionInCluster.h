#ifndef _DIMENSIONINCLUSTER_H_
#define _DIMENSIONINCLUSTER_H_

#include "cil/cilBase.h"
#include "utils/sizeSpan.h"

namespace FDU {
namespace cil_lib {

class dimenCluster : public CilBase {
public:
  enum direction { horizontal, vertical };

private:
  sizeSpan _normSizeSpan;
  sizeSpan _clusterSizeSpan;
  sizeSpan _arraySizeSpan;

public:
  explicit dimenCluster() : CilBase("dimension") {}

  void setSizeByDirect(direction direct, int cls_size, int nom_size);

  void setarrySizeSpan(const sizeSpan &arrySize) { _arraySizeSpan = arrySize; }
  sizeSpan &getArrySizeSpan() { return _arraySizeSpan; }
  sizeSpan &getNormSizeSpan() { return _normSizeSpan; }
  sizeSpan &getClusSizeSpan() { return _clusterSizeSpan; }
};

} // namespace cil_lib
} // namespace FDU

#endif