#ifndef MAPINSTANCE_H
#define MAPINSTANCE_H

#include <map>
#include <string>

namespace VL2XML_PARSER {

class MapInstance {
public:
  MapInstance();
  ~MapInstance();

  // about instance name mapping
  std::string get_real_name() { return RealName; }
  std::string get_mapped_name() { return MappedName; }
  void set_real_name(std::string name) { RealName = name; }
  void set_mapped_name(std::string name) { MappedName = name; }

  // about port name mapping
  std::string get_mapped_port_through_real_port(std::string name) {
    return PortsMap[name];
  }
  void insert_port_pair(std::string real, std::string mapped) {
    PortsMap.insert(make_pair(real, mapped));
  }

private:
  std::string RealName;
  std::string MappedName;

  std::map<std::string, std::string> PortsMap;
};
} // namespace VL2XML_PARSER

#endif