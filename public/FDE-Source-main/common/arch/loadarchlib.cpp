#include <boost/algorithm/string.hpp>
#include <sstream>
#include <vector>

#include "loadarchlib.hpp"
#include "zfstream.h"
#include <rapidxml/rapidxml_utils.hpp> // file

using namespace boost;
using namespace std;

namespace COS {
namespace XML {

Design *ArchLibHandler::load_design(xml_node *node, Design *design) {
  FPGADesign *arch =
      dynamic_cast<FPGADesign *>(NetlistHandler::load_design(node, design));
  ASSERT(arch, "parse failed. no current design");
  arch->rename(get_attribute(node, "name"));

  load_info(node->first_node("device_info"), arch);
  load_package(node->first_node("package"), arch);

  return arch;
}

/* new devide_info format
   <device_info scale="35,55" wire_timming="43.8816e-3,34.8561e-3"
   slice_per_tile="2" carry_chain="0;1" bram_col="0,54"/>
*/
void ArchLibHandler::load_info(xml_node *node, FPGADesign *arch) {
  ASSERT(node, "parse failed. no device_info");

  arch->set_scale(lexical_cast<Point>(get_attribute(node, "scale")));
  arch->set_slice_num(lexical_cast<int>(get_attribute(node, "slice_per_tile")));

  double R = 0., C = 0.;
  char d;
  istringstream iss(get_attribute(node, "wire_timming"));
  iss >> R >> d >> C;
  if (iss)
    arch->set_wire_timing(R, C);

  int n;
  vector<int> zpos;
  iss.clear();
  iss.str(get_attribute(node, "carry_chain")); // reset stream state and buffer
  while (iss >> n) {
    zpos.push_back(n);
    d = 0;
    iss >> d;
    if (d != ',') { // ';' or EOF
      arch->add_carry_chain(zpos);
      zpos.clear();
    }
  }

  iss.clear();
  iss.str(get_attribute(node, "bram_col"));
  while (iss >> n) {
    arch->add_bram_col(n);
    iss >> d;
  }
}

void ArchLibHandler::load_package(xml_node *node, FPGADesign *arch) {
  arch->set_package(get_attribute(node, "name"));
  foreach_child(pad_node, node, "pad") {
    string sname = get_attribute(pad_node, "name");
    Point pos = lexical_cast<Point>(get_attribute(pad_node, "pos"));
    arch->add_pad(sname, pos);
  }
}

Module *ArchLibHandler::load_module(xml_node *node, Library *lib) {
  ArchCell *cell =
      dynamic_cast<ArchCell *>(NetlistHandler::load_module(node, lib));
  ASSERT(cell, "parse failed. no current cell");
  if (xml_node *path_node = node->first_node("paths"))
    load_path(path_node, cell);
  return cell;
}

void ArchLibHandler::load_path(xml_node *node, ArchCell *cell) {
  foreach_child(path_node, node, "path") {
    string sfrom = get_attribute(path_node, "from");
    string sto = get_attribute(path_node, "to");
    string stype = get_attribute(path_node, "type");
    int type = stype.empty() ? 0 : lexical_cast<int>(stype);
    ArchPath *path = cell->create_path(sfrom, sto, type);
    if (xml_node *timing_node = path_node->first_node("sw_timing")) {
      double R = lexical_cast<double>(get_attribute(timing_node, "R"));
      double Cin = lexical_cast<double>(get_attribute(timing_node, "Cin"));
      double Cout = lexical_cast<double>(get_attribute(timing_node, "Cout"));
      double delay = lexical_cast<double>(get_attribute(timing_node, "delay"));
      path->set_timing(R, Cin, Cout, delay);
    }
  }
}

Port *ArchLibHandler::load_port(xml_node *node, Module *mod) {
  ArchPort *port =
      dynamic_cast<ArchPort *>(NetlistHandler::load_port(node, mod));
  ASSERT(port, "parse failed. port: no current port");
  SideType side = lexical_cast<SideType>(get_attribute(node, "side"));
  port->set_side(side);
  return port;
}

Instance *ArchLibHandler::load_instance(xml_node *node, Module *mod) {
  ArchInstance *inst =
      dynamic_cast<ArchInstance *>(NetlistHandler::load_instance(node, mod));
  ASSERT(inst, "parse failed. instance: no current instance");

  string svalue = get_attribute(node, "z");
  int zpos = svalue.empty() ? -1 : lexical_cast<int>(svalue);
  inst->set_zpos(zpos);

  svalue = get_attribute(node, "logic_pos");
  Point logic_pos = svalue.empty() ? Point() : lexical_cast<Point>(svalue);
  inst->set_logic_pos(logic_pos);

  svalue = get_attribute(node, "bit_pos");
  Point bit_pos = svalue.empty() ? Point() : lexical_cast<Point>(svalue);
  inst->set_bit_pos(bit_pos);

  svalue = get_attribute(node, "phy_pos");
  Point phy_pos = svalue.empty() ? Point() : lexical_cast<Point>(svalue);
  inst->set_phy_pos(phy_pos);

  return inst;
}

} // namespace XML
} // namespace COS

namespace ARCH {
using namespace COS::XML;
typedef rapidxml::xml_document<> xml_document;
typedef rapidxml::file<> xml_file;

NetlistHandler *ArchFactory::make_xml_read_handler() {
  return new ArchLibHandler;
}

/// load arch library from a XML file
FPGADesign *FPGADesign::loadArchLib(const string &file_name,
                                    CosFactory *factory) {
  CosFactory::pointer old_factory = CosFactory::set_factory(factory);
  std::unique_ptr<NetlistHandler> alh(
      CosFactory::instance().make_xml_read_handler());
  FPGADesign *fpga_design = new FPGADesign;
  xml_document doc;

  ifstrm ifs(file_name.c_str());
  xml_file file(ifs);
  doc.parse<0>(file.data());
  alh->load_design(doc.first_node(), fpga_design);

  _instance.reset(fpga_design);
  CosFactory::set_factory(old_factory);
  return fpga_design;
}
} // namespace ARCH
