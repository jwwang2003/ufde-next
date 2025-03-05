#ifndef RTCSTLOADHANDLER_H
#define RTCSTLOADHANDLER_H
#include "ptrcontainer.hpp"
#include "rtnetlist.hpp"
#include "xmlutils.h"

namespace FDU {
namespace RT {
using namespace COS;
using namespace XML;
using std::string;
class CstNet {
public:
  typedef PtrVector<PIP>::const_range_type const_pips_type;

  CstNet(const string &name) : _name(name) {}
  const string &name() const { return _name; }
  PIP *create_pip(const string &from, const string &to, const Point &pos,
                  PIP::pip_dir dir = PIP::unidir) {
    return _pips.add(new PIP(from, to, pos, dir));
  }
  size_t num_pips() const { return _pips.size(); }
  const_pips_type routing_pips() const { return _pips.range(); }

private:
  string _name;
  PtrVector<PIP> _pips;
};

class CstNets {
  friend class Router;

public:
  typedef PtrVector<CstNet>::const_range_type const_nets_type;
  void setDesignName(const string &name) { _designName = name; }
  const string &getDesignName() const { return _designName; }
  CstNet *creat_cstNet(const string &name) {
    return _cstNets.add(new CstNet(name));
  }
  const CstNet *find_net(const string &name) const { return nets().find(name); }
  const_nets_type nets() const { return _cstNets.range(); }

private:
  CstNets() {}
  string _designName;
  PtrVector<CstNet> _cstNets;
};

class RTCstLoadHandler {
public:
  RTCstLoadHandler(const string &cstFileName, CstNets *pCstNets)
      : _cstFileName(cstFileName), _pCstNets(pCstNets) {}
  void load();

private:
  void loadCstNets(xml_node *node);
  void loadCstNet(xml_node *node);
  void loadPip(xml_node *node, CstNet *net);
  string _cstFileName;
  CstNets *_pCstNets;
};
} // namespace RT
} // namespace FDU
#endif