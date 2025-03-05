#include "plc_device.h"
#include "arch/archlib.hpp"
#include "plc_factory.h"
#include "plc_utils.h"

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>

namespace FDU {
namespace Place {

using boost::lexical_cast;
using namespace std;
using namespace boost::adaptors;
using namespace ARCH;

//////////////////////////////////////////////////////////////////////////
// streams

static const char *site_type[] = {"SLICE",   "TBUF",    "IOB",       "BLOCKRAM",
                                  "GCLK",    "GCLKIOB", "DLL",       "VCC",
                                  "BUFGMUX", "BRAM16",  "MULT18X18", "DCM"};
static EnumStringMap<Site::SiteType> site_type_map(site_type);
std::istream &operator>>(std::istream &s, Site::SiteType &type) {
  try {
    type = site_type_map.readEnum(s);
  } catch (bad_cast &e) {
    type = Site::IGNORE;
    return s;
  }
  return s;
}
std::ostream &operator<<(std::ostream &s, Site::SiteType type) {
  return site_type_map.writeEnum(s, type);
}

//////////////////////////////////////////////////////////////////////////
// Site

/************************************************************************/
/* �жϸ�site����ռ�õ�inst���Ƿ���macro��Ԫ*/
/************************************************************************/
bool Site::has_macro() const {
  bool has = false;
  for (PLCInstance *inst : _occ_insts) {
    if (inst && inst->is_macro()) {
      has = true;
      break;
    }
  }
  return has;
}

/************************************************************************/
/* �жϸ�site����ռ�õ�inst���Ƿ���fix��Ԫ*/
/************************************************************************/
bool Site::has_fixed_insts() const {
  bool has = false;
  for (PLCInstance *inst : _occ_insts) {
    if (inst && inst->is_fixed()) {
      has = true;
      break;
    }
  }
  return has;
}

/************************************************************************/
/* �õ���site��macro ���� */
/************************************************************************/
FLOORPLAN::SwapableType Site::macro_type() const {
  FLOORPLAN::SwapableType type = FLOORPLAN::SITE;
  for (PLCInstance *inst : _occ_insts) {
    if (inst && inst->swapable_type() != FLOORPLAN::SITE) {
      type = inst->swapable_type();
      break;
    }
  }
  return type;
}

int Site::get_unocc_z_pos() const {
  Point logic_pos = _logic_pos;

  for (int z_pos = 0; z_pos < _occ_insts.size(); ++z_pos) {
    logic_pos.z = z_pos;
    if (_type == IOB) {
      if (!_occ_insts[z_pos] && FPGADesign::instance()->is_io_bound(logic_pos))
        return z_pos;
    } else if (!_occ_insts[z_pos])
      return z_pos;
  }

  return FLOORPLAN::INVALID_VALUE;
}

PLCInstance *Site::get_occ_inst(int idx) const {
  for (int i = 0; i < _occ_insts.size(); ++i) {
    if (_occ_insts[i]) {
      if (idx != 0)
        --idx;
      else if (idx == 0 && _occ_insts[i]->is_fixed())
        return nullptr;
      else
        return _occ_insts[i];
    }
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Tile

//////////////////////////////////////////////////////////////////////////
// DeviceInfo

DeviceInfo::DelayTables DeviceInfo::_delay_tables;
Matrix<Point> DeviceInfo::_phy_pos_lut;

double DeviceInfo::get_p2p_delay(PLCInstance *src, PLCInstance *sink) {
  Site::SiteType src_type = src->site_type();
  Site::SiteType sink_type = sink->site_type();

  int delta_row = abs(src->curr_logic_pos().x - sink->curr_logic_pos().x);
  int delta_col = abs(src->curr_logic_pos().y - sink->curr_logic_pos().y);

  double delay = DEVICE::DEFAULT_P2P_DELAY;
  if (src_type == Site::SLICE || src_type == Site::BLOCKRAM) {
    if (sink_type == Site::SLICE)
      delay = lookup_delay_from_table(_delay_tables[DEVICE::CLB_TO_CLB],
                                      delta_row, delta_col);
    else if (sink_type == Site::IOB)
      delay = lookup_delay_from_table(_delay_tables[DEVICE::CLB_TO_OUTPUT],
                                      delta_row, delta_col);
    else
      delay = lookup_delay_from_table(_delay_tables[DEVICE::CLB_TO_CLB],
                                      delta_row, delta_col);
  } else if (src_type == Site::IOB) {
    if (sink_type == Site::SLICE)
      delay = lookup_delay_from_table(_delay_tables[DEVICE::INPUT_TO_CLB],
                                      delta_row, delta_col);
    else if (sink_type == Site::IOB)
      delay = lookup_delay_from_table(_delay_tables[DEVICE::INPUT_TO_OUTPUT],
                                      delta_row, delta_col);
    else
      delay = lookup_delay_from_table(_delay_tables[DEVICE::INPUT_TO_CLB],
                                      delta_row, delta_col);
  } else {
    delay = DEVICE::DEFAULT_P2P_DELAY; // for GCLK, GCLKIOB, TBUF, DLL ...
  }

  return delay < 0. ? DEVICE::DEFAULT_P2P_DELAY : delay;
}

void DeviceInfo::sum_up_rsc_in_device() {
  Library *tile_lib =
      FPGADesign::instance()->find_or_create_library(LIBRARY::TILE);
  Property<RscStat> &site_infos =
      create_temp_property<RscStat>(COS::MODULE, CELL::SITE_INFO);
  for (Module *cell : tile_lib->modules()) {
    cell->set_property(site_infos, RscStat());
    RscStat &rsc_stat = *cell->property_ptr(site_infos);

    for (Instance *inst : cell->instances()) {
      Site::SiteType site_type =
          lexical_cast<Site::SiteType>(inst->module_type());
      if (is_logic_site(site_type)) {
        if (rsc_stat.count(site_type))
          rsc_stat[site_type] += 1;
        else
          rsc_stat[site_type] = 1;
      }
    }
  }

  _phy_pos_lut.renew(_scale.x, _scale.y);

  vector<int> temp_rsc_stat(Site::NUM_OF_SITE_TYPE, 0);
  for (int row = 0; row < _scale.x; ++row) {
    for (int col = 0; col < _scale.y; ++col) {
      ArchInstance *tile_inst =
          FPGADesign::instance()->get_inst_by_pos(Point(row, col));

      _phy_pos_lut.at(row, col) = tile_inst->phy_pos();

      RscStat &rsc_stat = *(tile_inst->down_module()->property_ptr(site_infos));
      for (RscStat::value_type &rsc : rsc_stat) {
        if (rsc.first == Site::IOB) {
          for (int z = 0; z < rsc.second; ++z) {
            if (FPGADesign::instance()->is_io_bound(Point(row, col, z)))
              ++temp_rsc_stat[rsc.first];
          }
        } else
          temp_rsc_stat[rsc.first] += rsc.second;
      }
    }
  }

  for (int i = 0; i < Site::NUM_OF_SITE_TYPE; ++i) {
    if (temp_rsc_stat[i])
      _rsc_stat[(Site::SiteType)i] = temp_rsc_stat[i];
  }
}

double DeviceInfo::lookup_delay_from_table(const DelayTable &table,
                                           int delta_row, int delta_col) {
  int rows = table.size().x, cols = table.size().y;

  delta_row = delta_row < 0 ? 0 : delta_row >= rows ? rows - 1 : delta_row;
  delta_col = delta_col < 0 ? 0 : delta_col >= cols ? cols - 1 : delta_col;

  return table.at(delta_row, delta_col);
}

} // namespace Place
} // namespace FDU