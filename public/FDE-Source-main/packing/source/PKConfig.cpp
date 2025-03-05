#include "PKConfig.h"
#include "PKUtils.h"
#include "xmlutils.h"
#include "zfstream.h"
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <rapidxml/rapidxml_utils.hpp> // file<>
#include <string>

namespace PACK {

using namespace std;
using namespace COS;
using namespace FDU::XML;
typedef rapidxml::file<> xml_file;

PKConfig::PKConfig(const string &inst_type, const string &name,
                   const string &xdl_name, bool has_inner,
                   const string &def_val)
    : config_(create_config(inst_type, name, def_val)), xdl_name_(xdl_name),
      has_inner_(has_inner) {}

void PKConfig::xdl_out(ostream &os, Instance *inst) {
  string inner;
  string pval = inst->property_value(config_);
  ASSERTSD(!pval.empty());
  if (has_inner_ && xdl_name_[0] >= 'U' &&
      pval == CFG_NAME::DEF_VALUE) // fdp4pro
    return;
  if (has_inner_ && pval != CFG_NAME::DEF_VALUE)
    inner = inst->property_value(
        create_temp_property<string>(INSTANCE, xdl_name_ + "NAME"));
  os << "\t\t" << xdl_name_ << ':' << inner << ':' << pval << "\n";
}

void ConfigInfo::insert(const value_type &val) {
  PKConfig *cfg = 0;
  switch (val.type_) {
  case NORMAL:
    cfg = new PKConfig(cur_type_, val.name_, val.name_);
    break;
  case RAM_INIT:
    cfg = new PKConfig(cur_type_, boost::to_upper_copy(val.name_), val.name_);
    break;
  case LUT:
    ASSERT(cur_type_ == CFG_NAME::SLICE, "block type error, SLICE required");
    cfg = new PKConfig(cur_type_, val.name_, val.name_, true);
    break;
  }
  ASSERT(cfg, "Invalid configure type.");
  config_map_[cur_type_].add(cfg);
}

class ConfigLoader { // added by hz
private:
  ConfigInfo *owner;

public:
  ConfigLoader(ConfigInfo *conf) : owner(conf) {}
  void parse(istream &ifs);
  void load_library(xml_node *node);
  void load_block(xml_node *node);
  void load_primitive(xml_node *node);
  void load_config(xml_node *node);
};

void ConfigLoader::parse(istream &ifs) {
  xml_file f(ifs); // f指向fdp_dc_plib.xml
  xml_document doc;
  doc.parse<0>(f.data()); // rapidxml中提供的操作，解析fdp_dc_plib.xml网表
  xml_node *root = doc.first_node();
  load_block(root);
}

void ConfigLoader::load_library(xml_node *node) {}

void ConfigLoader::load_block(xml_node *node) {
  foreach_child(blk_node, node, "block") {
    string blockname = get_attribute(blk_node, "name");
    string btemp = owner->using_type(boost::to_upper_copy(blockname));
    foreach_child(prm_node, blk_node, "primitive") load_primitive(prm_node);

#ifdef _POWER
    if (btemp == CFG_NAME::SLICE) {
      owner->insert(ConfigInfo::value_type(
          "F_TRUTHTABLE", ConfigInfo::LUT)); // for power analysis
      owner->insert(ConfigInfo::value_type("G_TRUTHTABLE", ConfigInfo::LUT));
      owner->insert(ConfigInfo::value_type("U_TRUTHTABLE", ConfigInfo::LUT));
      owner->insert(ConfigInfo::value_type("V_TRUTHTABLE", ConfigInfo::LUT));
    }
#endif
  }
}

void ConfigLoader::load_primitive(xml_node *node) {
  string prmname = get_attribute(node, "name");
  string prmtype = get_attribute(node, "type");
  ConfigInfo::cfg_type prm;
  if (prmtype.empty())
    prm = ConfigInfo::NORMAL;
  else if (prmtype == "LUT")
    prm = ConfigInfo::LUT;
  else if (prmtype == "any")
    return;
  else
    prm = ConfigInfo::RAM_INIT;
  owner->insert(ConfigInfo::value_type(prmname, prm));
  // #ifdef _POWER
  // owner->insert(ConfigInfo::value_type("TRUTHTABLE"));//for power analysis
  // #endif
}

void ConfigInfo::load(const std::string &fg) { // added by hz
  ConfigLoader loader(this);
  ifstrm ifgs(fg.c_str());
  loader.parse(ifgs);
}

} // namespace PACK