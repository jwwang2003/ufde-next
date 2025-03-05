#include "bitstream/memLibBstr/dfotMemLibBstr/dfotMemBstr/dfotMemBstr.h"
#include "container/Container.h"

namespace BitGen {
namespace bitstream {

class dfotMemLibBstr {
public:
  using dfotMemsType = bstrContainer<dfotMemBstr>::range_type;
  using const_dfotMemsType = bstrContainer<dfotMemBstr>::const_range_type;
  using dfotMemIter = bstrContainer<dfotMemBstr>::iterator;
  using const_dfotMemIter = bstrContainer<dfotMemBstr>::const_iterator;

private:
  bstrContainer<dfotMemBstr> _dfotMems;

public:
  //		dfotMemLibBstr() {}

  dfotMemsType dfotMems() { return _dfotMems.range(); }
  const_dfotMemsType dfotMems() const { return _dfotMems.range(); }

  dfotMemBstr *addDfotMem(dfotMemBstr *dfotMem) {
    return _dfotMems.add(dfotMem);
  }
  dfotMemBstr &getDfotMem() { return **_dfotMems.begin(); }

  void construct();
};

} // namespace bitstream
} // namespace BitGen