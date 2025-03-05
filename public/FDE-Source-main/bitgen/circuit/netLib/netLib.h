#ifndef _NETLIB_H_
#define _NETLIB_H_

#include "circuit/netLib/net/net.h"

namespace BitGen {
namespace circuit {

class netLib {
public:
  using netsType = cktContainer<Net>::range_type;
  using const_netsType = cktContainer<Net>::const_range_type;
  using netIter = cktContainer<Net>::iterator;
  using const_netIter = cktContainer<Net>::const_iterator;

private:
  cktContainer<Net> _nets;
  instLib *_refInstLib;

public:
  explicit netLib(instLib *refInstLib = 0) : _refInstLib(refInstLib) {}

  netsType nets() { return _nets.range(); }
  const_netsType nets() const { return _nets.range(); }

  Net *addNet(Net *net) { return _nets.add(net); }
  Net &getNet(const std::string &netName) { return *nets().find(netName); }

  void listNetPips(vecPips &pips);
};

} // namespace circuit
} // namespace BitGen

#endif