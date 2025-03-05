#ifndef _NET_H_
#define _NET_H_

#include "circuit/cktBase.h"
#include "circuit/instLib/instLib.h"
#include "circuit/netLib/net/pinInNet.h"
#include "circuit/netLib/net/pipInNet.h"

namespace BitGen {
namespace circuit {

class Net : public cktBase {
private:
  contPinsNet _opins;
  contPinsNet _ipins;
  contPipsNet _pips;
  instLib *_refInstLib;
  FPGADesign *_archLibrary;

public:
  // for XDL
  Net(xdlMatch *matchResults, instLib *refInstLib = 0)
      : cktBase("", matchResults), _refInstLib(refInstLib) {
    constructFromXDL();
  }
  // for XML
  Net(FPGADesign *archLibrary = 0, COS::Object *net = 0,
      instLib *refInstLib = 0)
      : cktBase(net), _archLibrary(archLibrary), _refInstLib(refInstLib) {
    constructFromXML();
  }

  pinNet *addOPin(pinNet *opin) { return _opins.addPin(opin); }
  pinNet *addIPin(pinNet *ipin) { return _ipins.addPin(ipin); }
  pipNet *addPip(pipNet *pip) { return _pips.addPip(pip); }

  instLib *getInstLib() const { return _refInstLib; }

  void listNetPips(vecPips &pips);

  virtual void constructFromXDL(const std::string &info = "");
  virtual void constructFromXML();
};

class parsePinPip {
private:
  Net *_curNet;

public:
  explicit parsePinPip(Net *curNet) : _curNet(curNet) {}

  bool operator()(const cktBase::xdlMatch &what) const;
};

//////////////////////////////////////////////////////////////////////////
// inline functions

inline void Net::listNetPips(vecPips &pips) { _pips.listPips(pips); }

} // namespace circuit
} // namespace BitGen

#endif