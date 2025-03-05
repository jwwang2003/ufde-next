#ifndef RTNETLIST_HPP
#define RTNETLIST_HPP

#include "tnetlist.hpp"

namespace COS {

///////////////////////////////////////////////////////////////////////////////////////
//  RoutingConnection

class PIP {
public:
  enum pip_dir { unidir, bidir_nobuffer, bidir_onebuffer, bidir_bothbuffer };
  PIP(const string &from, const string &to, const Point &pos, pip_dir dir)
      : _from_port(from), _to_port(to), _position(pos), _dir(dir) {}

  const string &from() const { return _from_port; }
  const string &to() const { return _to_port; }
  const Point &position() const { return _position; }
  pip_dir dir() const { return _dir; }

private:
  string _from_port;
  string _to_port;
  Point _position;
  pip_dir _dir;
};

static const char *pip_dirs[] = {"->", "==", "=>", "=-"};
static EnumStringMap<PIP::pip_dir> pipmap(pip_dirs);
inline std::istream &operator>>(std::istream &s, PIP::pip_dir &dir) {
  dir = pipmap.readEnum(s);
  return s;
}
inline std::ostream &operator<<(std::ostream &s, PIP::pip_dir dir) {
  return pipmap.writeEnum(s, dir);
}

///////////////////////////////////////////////////////////////////////////////////////
//  RTNet
class COSRTNet : public Net {
  PtrVector<PIP> _pips;

public:
  COSRTNet(const string &name, NetType type, Module *owner, Bus *bus)
      : Net(name, type, owner, bus) {}

  using pips_type = PtrVector<PIP>::range_type;
  using const_pips_type = PtrVector<PIP>::const_range_type;
  using pip_iter = PtrVector<PIP>::iterator;
  using const_pip_iter = PtrVector<PIP>::const_iterator;
  PIP *create_pip(const string &from, const string &to, const Point &pos,
                  PIP::pip_dir dir = PIP::unidir) {
    return _pips.add(new PIP(from, to, pos, dir));
  }
  std::size_t num_pips() const { return _pips.size(); }
  pips_type routing_pips() { return _pips.range(); }
  const_pips_type routing_pips() const { return _pips.range(); }
  void clear_pips() { _pips.clear(); }
};

///////////////////////////////////////////////////////////////////////////////////////
//  RTModule
class RTModule : public Module {
public:
  RTModule(const string &name, const string &type, Library *owner)
      : Module(name, type, owner) {}
  //  Nets
  using nets_type = PtrVector<Net>::typed<COSRTNet>::range;
  using const_nets_type = PtrVector<Net>::typed<COSRTNet>::const_range;
  using net_iter = PtrVector<Net>::typed<COSRTNet>::iterator;
  using const_net_iter = PtrVector<Net>::typed<COSRTNet>::const_iterator;

  nets_type nets() { return static_cast<nets_type>(Module::nets()); }
  const_nets_type nets() const {
    return static_cast<const_nets_type>(Module::nets());
  }

  COSRTNet *find_net(const string &name) { return nets().find(name); }
  const COSRTNet *find_net(const string &name) const {
    return nets().find(name);
  }
};

///////////////////////////////////////////////////////////////////////////////////////
//  RTFactory
class COSRTFactory : public TimingFactory {
public:
  virtual Net *make_net(const string &name, NetType type, Module *owner,
                        Bus *bus) {
    return new COSRTNet(name, type, owner, bus);
  }
  virtual Module *make_module(const string &name, const string &type,
                              Library *owner) {
    return new RTModule(name, type, owner);
  }

  virtual XML::NetlistHandler *make_xml_read_handler();
  virtual XML::NetlistWriter *make_xml_write_handler();
};

} // namespace COS

#endif
