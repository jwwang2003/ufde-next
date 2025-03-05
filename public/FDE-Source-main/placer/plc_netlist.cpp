#include "plc_netlist.h"
#include "arch/archlib.hpp"
#include "plc_factory.h"
#include "plc_utils.h"
#include <boost/lexical_cast.hpp>

#include <fstream>
#include <iostream>
#include <string>

#include "report/report.h"

namespace FDU {
namespace Place {

using boost::lexical_cast;
using namespace boost;
using namespace std;
using namespace ARCH;
using namespace rapidxml;
//////////////////////////////////////////////////////////////////////////
// SwapObject
/************************************************************************/
/* �õ�һ��swapobj���߼���ַ */
/************************************************************************/
Point SwapObject::base_logic_pos() const {
  switch (_type) {
  case FLOORPLAN::SITE:
    return _insts[0]->curr_logic_pos();
  case FLOORPLAN::LUT6:
    return _insts[0]->curr_logic_pos();
  case FLOORPLAN::CARRY_CHAIN:
    return _insts[0]->curr_logic_pos();
  default:
    ASSERT(0, (CONSOLE::PLC_ERROR % "unknown swap object."));
  }
}
/************************************************************************/
/* �õ���swap obj��site                                                 */
/************************************************************************/
Site &SwapObject::base_loc_site() const {
  switch (_type) {
  case FLOORPLAN::SITE:
    return *_insts[0]->curr_loc_site();
  case FLOORPLAN::LUT6:
    return *_insts[0]->curr_loc_site();
  case FLOORPLAN::CARRY_CHAIN:
    return *_insts[0]->curr_loc_site();
  default:
    ASSERT(0, (CONSOLE::PLC_ERROR % "unknown swap object."));
  }
}
/************************************************************************/
/* ���ܣ�����һ��swapobj���߼�λ�ú͵�Ԫ����
 * ������
 *		inst_idx��int��Ҫ�޸ĵ�instance��index��
 *		pos:Point��Ҫ���µ�Ŀ���߼�λ��
 *		loc_site:Site��Ҫ���µ�Ŀ��site
 */
/************************************************************************/
void SwapObject::update_place_info(int inst_idx, const Point &pos,
                                   Site &loc_site) {
  _insts[inst_idx]->set_curr_logic_pos(pos);
  _insts[inst_idx]->set_curr_loc_site(&loc_site);
}

//////////////////////////////////////////////////////////////////////////
// NLInfo
/************************************************************************/
/* ���ܣ�����Ƿ�ͬһ��λ������������Ԫ
 * ������void
 * ����ֵ��void
 * ˵����GCLKIOB����
 */
/************************************************************************/
void NLInfo::check_place() {
  vector<map<Point, PLCInstance *>> pos_checker(Site::NUM_OF_SITE_TYPE);
  // �������е�ÿһ��inst����Ƿ���λ���غ�
  for (PLCInstance *inst :
       static_cast<PLCModule *>(_design->top_module())->instances()) {
    Site::SiteType site_type =
        lexical_cast<Site::SiteType>(inst->module_type());
    // ����GCKIOB
    if (site_type == Site::GCLKIOB || site_type == Site::VCC)
      continue;
    // ����Ƿ���ڷǷ�inst����û��pack�ɹ���inst
    ASSERT(site_type != Site::IGNORE,
           (CONSOLE::PLC_ERROR % (inst->name() + ": invalid instance type.")));
    // ����Ƿ����غ�
    ASSERT(!pos_checker[site_type].count(inst->curr_logic_pos()),
           (CONSOLE::PLC_ERROR % (inst->name() + ": overlap position.")));
    // û���غϵĻ�ֱ�����úø�map vector
    pos_checker[site_type][inst->curr_logic_pos()] = inst;
  }
}
/************************************************************************/
/* ���ܣ����沼�ֵ���Ϣ
 * ������void
 * ����ֵ��void
 * ˵����GCLKIOB���⴦��
 */
/************************************************************************/
void NLInfo::save_place() {
  Property<Point> &positions =
      create_property<Point>(COS::INSTANCE, INSTANCE::POSITION);
  for (PLCInstance *inst :
       static_cast<PLCModule *>(_design->top_module())->instances()) {
    Site::SiteType site_type =
        lexical_cast<Site::SiteType>(inst->module_type());
    // ����GCLK
    if (site_type == Site::GCLKIOB) {
      Pin *gclkout_pin = inst->pins().find(DEVICE::GCLKOUT);
      ASSERT(gclkout_pin, (CONSOLE::PLC_ERROR %
                           (inst->name() + ": can NOT place GCLKIOB.")));
      PLCInstance *gclkbuf = static_cast<PLCInstance *>(
          gclkout_pin->net()->sink_pins().begin()->owner());
      inst->set_curr_logic_pos(gclkbuf->curr_logic_pos());
    }
    // ����ÿһ��instance��position��Ϣ

    inst->set_property(positions, inst->curr_logic_pos());
  }
  /************************************************************************/
  /*����V2�е�VCC���������ISE������*/
  /************************************************************************/
  for (PLCNet *net : static_cast<PLCModule *>(_design->top_module())->nets()) {
    PLCNet::pin_iter vcc_pin = find_if(net->pins(), [](const Pin *pin) {
      return pin->name() == DEVICE::VCCOUT;
    });
    if (vcc_pin != net->pins().end()) {
      PLCInstance *VCC = static_cast<PLCInstance *>(vcc_pin->owner());
      ASSERT(lexical_cast<Site::SiteType>(VCC->module_type()) == Site::VCC,
             (CONSOLE::PLC_ERROR % (VCC->name() + ": is not a VCC device")));
      PLCInstance *inst =
          static_cast<PLCInstance *>(net->sink_pins().begin()->owner());
      Point invalid_pos;
      Point pos = inst->property_value(positions);
      ASSERT(pos != invalid_pos,
             (CONSOLE::PLC_ERROR % (inst->name() + ": is not placed")));
      ArchCell *tile_cell =
          FPGADesign::instance()->get_inst_by_pos(pos)->down_module();
      ASSERTD(find_if(tile_cell->instances(),
                      [](const ArchInstance *inst) {
                        return inst->module_type() ==
                               lexical_cast<string>(Site::VCC);
                      }) != tile_cell->instances().end(),
              (CONSOLE::PLC_ERROR %
               ("VCC site does not exist at:" + lexical_cast<string>(pos))));
      VCC->set_property(positions, pos);
    }
  }
  // 		for (PLCNet& net:
  // static_cast<PLCModule&>(_design->top_cell()).nets())
  // net.clear_pips();
}
/************************************************************************/
/* ���ܣ�ͳ���õ���Դ
 * ������void
 * ����ֵ��void
 * ˵����
 */
/************************************************************************/
void NLInfo::classify_used_resource() {
  for (PLCInstance *inst :
       static_cast<PLCModule *>(_design->top_module())->instances()) {
    // ����Ѿ���һ������ô������+1
    if (_rsc_stat.count(inst->module_type()))
      _rsc_stat[inst->module_type()] += 1;
    // ��ǰû�У���ôΪ1
    else
      _rsc_stat[inst->module_type()] = 1;
  }

#ifdef UNUSED
  int carrychain_count = 0;
  for (MacroInfo::CarryChains::value_type &chains : _macro_info._carrychains)
    carrychain_count += chains.second.size();

  if (carrychain_count)
    _rsc_stat[Module::CARRY_CHAIN] = carrychain_count;
#endif
}
/************************************************************************/
/* ���ܣ�ͳ�������е���Դ��Ϣ
 * ������void
 * ����ֵ��void
 * ˵����
 */
/************************************************************************/
void NLInfo::echo_netlist_resource() {
  // design name
  INFO(CONSOLE::DESIGN % _design->name());
  // ���ÿһ�����͵���ľ
  for (RscStat::value_type &rsc : _rsc_stat)
    INFO(CONSOLE::RSC_IN_DESIGN % rsc.first % rsc.second);
  // ���������Ŀ
  INFO(CONSOLE::RSC_IN_DESIGN % "Net" % _design->top_module()->num_nets());
}
/************************************************************************/
/* ���ܣ������Դ������
 * ������void
 * ����ֵ��void
 * ˵����
 */
/************************************************************************/
void NLInfo::echo_resource_usage(DeviceInfo::RscStat &dev_rsc) {
  // ��������
  INFO(CONSOLE::DEVICE_TYPE % ARCH::FPGADesign::instance()->name());
  // ���ÿһ���͵Ķ������õ�percent
  for (const RscStat::value_type &rsc : _rsc_stat) {
    Site::SiteType type = lexical_cast<Site::SiteType>(rsc.first);

    if (dev_rsc.count(type)) {
      double percent = (double)rsc.second / dev_rsc[type] * 100.;
      if (type == Site::SLICE)
        INFO(CONSOLE::RSC_SLICE % rsc.first % DEVICE::NUM_LUT_INPUTS % percent);
      else
        INFO(CONSOLE::RSC_USAGE % rsc.first % percent);
    }
  }
}
/************************************************************************/
/* ���ܣ�ͳ�������е���Դ��Ϣ
 * ������void
 * ����ֵ��void
 * ˵����
 */
/************************************************************************/

void NLInfo::writeReport(DeviceInfo::RscStat &dev_rsc, string output_file) {
  using namespace FDU::RPT;
  char val_string[25];
  char percent_string[25];
  double percent;

  string design_name = output_file;
  int str_len = design_name.length();
  design_name = design_name.replace(str_len - 8, 8, "");
  Report *rpt = new Report();
  rpt->set_design(design_name);

  Section *sec_type = rpt->create_section("Type_Count", "Type Count");
  Table *table_type = sec_type->create_table("General_Table");
  table_type->create_column("Type_Name", "Type Name");
  table_type->create_column("Count", "Count");

  for (RscStat::value_type &rsc : _rsc_stat) {
    Site::SiteType type = lexical_cast<Site::SiteType>(rsc.first);
    if (dev_rsc.count(type)) {
      percent = (double)rsc.second / dev_rsc[type] * 100.;
      snprintf(percent_string, 25, "%.2f%%", percent);
      snprintf(val_string, 25, "%d", rsc.second);

      Row *row = table_type->create_row();
      row->set_item("Type_Name", rsc.first);
      row->set_item("Count", rsc.second);
    }
  }

  rpt->write(design_name + "_plc_rpt.xml");
}

} // namespace Place
} // namespace FDU