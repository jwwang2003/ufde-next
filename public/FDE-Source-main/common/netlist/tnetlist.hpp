#ifndef TNETLIST_HPP
#define TNETLIST_HPP

#include "netlist.hpp"
#include "tengine/tengine.hpp"

namespace COS {

///////////////////////////////////////////////////////////////////////////////////////
//  Design
class TDesign : public Design {
public:
  explicit TDesign(const string &name = "") : Design(name), _tengine(this) {}

  double timing_analyse(TEngine::WORK_MODE mode = TEngine::INCREMENT) {
    return _tengine.timing_analyse(mode);
  }

private:
  TEngine _tengine;
};

///////////////////////////////////////////////////////////////////////////////////////
//  TPort
class TPort : public Port {
  vector<TimingInfo> _timings;
  double _C;

public:
  TPort(const string &name, int msb, int lsb, DirType dir, PortType type,
        bool is_vec, Module *owner, int pin_from)
      : Port(name, msb, lsb, dir, type, is_vec, owner, pin_from), _C(0.0) {}

  // Timings, readonly
  using const_timing_iter = vector<TimingInfo>::const_iterator;
  using const_timings_type = boost::iterator_range<const_timing_iter>;

  void set_capacitance(double C) { _C = C; }
  double capacitance() const { return _C; }

  void create_timing(const string &type, const string &pin, const string &sense,
                     double rise, double fall) {
    return _timings.push_back(TimingInfo(type, pin, sense, rise, fall));
  }
  void create_timing(const TimingInfo &tinfo) {
    return _timings.push_back(tinfo);
  }
  std::size_t num_timings() const { return _timings.size(); }
  const_timings_type timings() const { return const_timings_type(_timings); }

protected:
  Object *do_clone(Object *new_owmer) const {
    TPort *new_port = dynamic_cast<TPort *>(Port::do_clone(new_owmer));
    for (const TimingInfo &tinfo : _timings)
      new_port->create_timing(tinfo);
    return new_port;
  }
};

///////////////////////////////////////////////////////////////////////////////////////
//  TimingFactory
class TimingFactory : public CosFactory {
public:
  virtual Port *make_port(const string &name, int msb, int lsb, DirType dir,
                          PortType type, bool is_vec, Module *owner,
                          int pin_from) {
    return new TPort(name, msb, lsb, dir, type, is_vec, owner, pin_from);
  }

  virtual XML::NetlistHandler *make_xml_read_handler();
  virtual XML::NetlistWriter *make_xml_write_handler();
};

} // namespace COS

#endif
