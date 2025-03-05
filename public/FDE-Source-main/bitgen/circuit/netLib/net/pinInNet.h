#ifndef _PININNET_H_
#define _PININNET_H_

#include "circuit/cktBase.h"
#include "circuit/instLib/instLib.h"

namespace BitGen {
namespace circuit {

class pinNet : public cktBase {
private:
  std::string _instName;
  instLib *_refInstLib;

public:
  pinNet(const std::string &info, instLib *refInstLib = 0)
      : cktBase("", nullptr), _refInstLib(refInstLib) {
    constructFromXDL(info);
  }

  pinNet(const std::string &pinName, const std::string &instName,
         instLib *refInstLib = 0)
      : _instName(instName), _refInstLib(refInstLib) {
    _name = pinName;
  }

  virtual void constructFromXDL(const std::string &info);
  virtual void constructFromXML() {}
};

class contPinsNet {
public:
  using pinsNetType = cktContainer<pinNet>::range_type;
  using const_pinsNetType = cktContainer<pinNet>::const_range_type;
  using pinNetIter = cktContainer<pinNet>::iterator;
  using const_pinNetIter = cktContainer<pinNet>::const_iterator;

private:
  cktContainer<pinNet> _pins;
  instLib *_refInstLib;

public:
  void setRefInstLib(instLib *ref) { _refInstLib = ref; }

  pinsNetType pins() { return _pins.range(); }
  const_pinsNetType pins() const { return _pins.range(); }

  pinNet *addPin(pinNet *pin) { return _pins.add(pin); }
  pinNet &getPin(const std::string &pinName) { return *pins().find(pinName); }
};

} // namespace circuit
} // namespace BitGen

#endif