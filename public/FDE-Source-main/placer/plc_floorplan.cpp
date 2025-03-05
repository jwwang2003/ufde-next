#include "plc_floorplan.h"
#include "arch/archlib.hpp"
#include "plc_args.h"
#include "plc_const_infer.h"
#include "plc_utils.h"
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/tuple/tuple.hpp>

// #include "report.h"
// #include "reportmanager.hpp"

namespace FDU {
namespace Place {

using boost::lexical_cast;
using namespace std;
using namespace boost;
// using namespace boost::lambda;
//	using namespace boost::adaptors;
using namespace ARCH;

/************************************************************************/
/*	���ܣ�Ϊ����ģ���˻��������׼��
 *	������void
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
bool Floorplan::ready_for_place(string output_file) {
  // ����������ģ
  bool isAllFixed;
  _dev_info.set_device_scale(FPGADesign::instance()->scale());
  // ����ÿ��tile�е�silice��Ŀ
  _dev_info.set_slice_num(FPGADesign::instance()->get_slice_num());
  _dev_info.set_carry_num(FPGADesign::instance()->get_carry_num());
  _dev_info.set_carry_chain(FPGADesign::instance()->get_carry_chain());
  _dev_info.set_lut_inputs(FPGADesign::instance()->get_LUT_inputs());
  vector<vector<int>> a = DEVICE::carry_chain;
  // ͳ��������Ϣ
  _nl_info.classify_used_resource();
  // ͳ������������Դ
  _dev_info.sum_up_rsc_in_device();
  // ���������Դ��Ϣ
  _nl_info.echo_netlist_resource();
  _nl_info.echo_resource_usage(_dev_info.rsc_in_device());
  // ���report
  _nl_info.writeReport(_dev_info.rsc_in_device(), output_file);
  // ����������һ��FPGA
  INFO(CONSOLE::PROGRESS % "50" % "build FPGA architecture");
  build_fpga();
  // ��ʼ������
  INFO(CONSOLE::PROGRESS % "60" % "begin to initially place");
  isAllFixed = init_place();
  return isAllFixed;
}
/************************************************************************/
/*	���ܣ�����һ��FPGA
 *	������void
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
void Floorplan::build_fpga() {
  Property<DeviceInfo::RscStat> &site_infos =
      create_temp_property<DeviceInfo::RscStat>(COS::MODULE, CELL::SITE_INFO);
  int rows = _dev_info.device_scale().x;
  int cols = _dev_info.device_scale().y;
  //_fpga��һ��Tile����
  _fpga.renew(rows, cols);

  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      ArchInstance *tile_inst =
          FPGADesign::instance()->get_inst_by_pos(Point(row, col));
      ArchCell *tile_cell = tile_inst->down_module();

      Tile &tile = _fpga.at(row, col);
      tile.set_type(tile_cell->name());
      tile.set_phy_pos(tile_inst->phy_pos());

      DeviceInfo::RscStat &rsc_stat = *tile_cell->property_ptr(site_infos);
      for (DeviceInfo::RscStat::value_type &rsc : rsc_stat) {
        Site &site = tile.add_site(rsc.first);
        if (site._type == Site::IOB) {
          for (int z = 0; z < rsc.second; ++z) {
            if (FPGADesign::instance()->is_io_bound(Point(row, col, z)))
              ++site._capacity;
          }
        } else if (site._type == Site::BLOCKRAM) {
          if (tile_cell->name() == DEVICE::LBRAMD ||
              tile_cell->name() == DEVICE::RBRAMD)
            ++site._capacity;
          else
            site._capacity = 0;
        } else
          site._capacity = rsc.second;
        site._occ_insts.resize(rsc.second, nullptr);
        site._logic_pos = Point(row, col);
        if (site._capacity)
          _dev_info.add_rsc_ava_pos(site._type, Point(row, col));
      }
    }
  }

  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      for (Tile::Sites::value_type &type_site_map :
           _fpga.at(row, col).sites()) {
        Site *site = type_site_map.second;
        vector<Point> &ava_pos = _dev_info.rsc_ava_pos(site->_type);
        site->_available_pos.assign(ava_pos.begin(), ava_pos.end());
      }
    }
  }
}

/************************************************************************/
/*	���ܣ���ʼ������
 *	������void
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
bool Floorplan::init_place() {
#ifdef CONST_GENERATE
  ConstantsGener const_gener;
  const_gener.generate_constants(_nl_info.design());
#endif
  bool isAllFixed;
  // ���ͱ���Լ��
  check_and_save_cst();
  // �ҵ�design�����е�carry chain
  CarryChainInference::CarryChains carrys;
  CarryChainInference carry_infer;
  carry_infer.inference(_nl_info.design(), carrys);
  // �ҵ�design�����е�Lut6
  LUT6Inference::LUT6s lut6s;
  LUT6Inference lut6_infer;
  lut6_infer.inference(_nl_info.design(), lut6s);

  // ��ʼ������carry chain
  init_place_carrychain(carrys);
  // ��ʼ������lut6
  init_place_lut6(lut6s);
  // ��ʼ������site
  isAllFixed = init_place_site();
  // ���ÿ��net���Ƿ�����Ϊignored
  set_ignored_nets();
  // �洢ÿ��net���ӵ�instance,����ignored net���洢
  for (PLCNet *net :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->nets())
    net->store_connected_insts();
  // ��ʼ����������Ժ󣬸�����Ϣ���������ݿ�update_qualified_pos_for_carrychain����
  for (int col = 0; col < _dev_info.device_scale().y; ++col) {
    for (NLInfo::MacroInfo::CarryChains::value_type &chain :
         _nl_info.carry_chains()) {
      for (SwapObject *obj : chain.second)
        obj->init_qualified_pos_tile_col(col);
    }
    for (int index = 0; index < DEVICE::NUM_CARRY_PER_TILE; ++index)
      update_qualified_pos_for_carrychain(col, index);
  }
  return isAllFixed;
}

/************************************************************************/
/*	���ܣ���ʼ��carry chain
 *	������carrys��CarryChains�������е�carry chain����
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
void Floorplan::init_place_carrychain(
    CarryChainInference::CarryChains &carrys) {
  // ���������ҵ���ÿһ��carry chain
  for (CarryChainInference::CarryChains::value_type &carry : carrys) {
    // ��������Ϣ��������һ���ɽ�������
    SwapObject *obj = _nl_info.add_swap_object(FLOORPLAN::CARRY_CHAIN);
    // ���ÿɽ������������
    obj->fill(carry.second);
    // Ϊ��������һ��carry chain�Ŀɽ�������
    _nl_info.add_carry_swap_obj(obj);

    vector<Point> chain_pos_vec;
    const int MAX_LOOP_NUM = 5;
    int loop_count = 0;
    // �ҵ�һ����ʼ��λ��
    while (!find_init_pos_for_carrychain(carry.second.size(), chain_pos_vec)) {
      ASSERT(++loop_count < MAX_LOOP_NUM,
             (CONSOLE::PLC_ERROR %
              (carry.first + ": can NOT place carry chain.")));
    }

    ASSERT(chain_pos_vec.size() == carry.second.size(),
           (CONSOLE::PLC_ERROR % "illegal found positions for carry chain."));

    for (int i = 0; i < chain_pos_vec.size(); ++i) {
      Point pos = chain_pos_vec[i];
      PLCInstance *inst = carry.second[i];
      Site &site = _fpga.at(pos).site(Site::SLICE);

      ++site._occ;
      site._occ_insts[pos.z] = inst;

      inst->set_swapable_type(FLOORPLAN::CARRY_CHAIN);
      inst->set_curr_logic_pos(pos);
      inst->set_curr_loc_site(&site);
    }
  }
}

void Floorplan::init_place_lut6(LUT6Inference::LUT6s &lut6s) {
  typedef pair<PLCInstance *, PLCInstance *> LUT6;

  vector<Point> &ava_pos = _dev_info.rsc_ava_pos(Site::SLICE);

  for (LUT6 &lut6 : lut6s) {
    int rand_idx;
    Site *slice_site = nullptr;
    do {
      rand_idx = rand() % ava_pos.size();
      slice_site = &_fpga.at(ava_pos[rand_idx]).site(Site::SLICE);
    } while (slice_site->_occ != 0);

    PLCInstance *src_slice = lut6.first;
    PLCInstance *sink_slice = lut6.second;

    slice_site->_occ_insts[0] = src_slice;
    slice_site->_occ_insts[1] = sink_slice;
    slice_site->_occ = DEVICE::NUM_SLICE_PER_TILE;

    Point pos = ava_pos[rand_idx];
    src_slice->set_curr_logic_pos(Point(pos.x, pos.y, 0));
    sink_slice->set_curr_logic_pos(Point(pos.x, pos.y, 1));

    src_slice->set_curr_loc_site(slice_site);
    sink_slice->set_curr_loc_site(slice_site);

    src_slice->set_swapable_type(FLOORPLAN::LUT6);
    sink_slice->set_swapable_type(FLOORPLAN::LUT6);

    SwapObject *obj = _nl_info.add_swap_object(FLOORPLAN::LUT6);
    obj->fill(src_slice);
    obj->fill(sink_slice);

    _dev_info.pop_rsc_ava_pos(Site::SLICE, rand_idx);
  }
}

bool Floorplan::init_place_site() {
  for (PLCInstance *inst :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->instances()) {
    Site::SiteType site_type =
        lexical_cast<Site::SiteType>(inst->module_type());

    if (!_dev_info.is_logic_site(site_type) || site_type == Site::GCLKIOB ||
        inst->is_fixed() || inst->is_macro() || site_type == Site::VCC)
      continue;

    vector<Point> &ava_pos = _dev_info.rsc_ava_pos(site_type);
    int rand_idx = rand() % ava_pos.size();
    Site *site = &_fpga.at(ava_pos[rand_idx]).site(site_type);

    while (site->_occ >= site->_capacity) {
      _dev_info.pop_rsc_ava_pos(site_type, rand_idx);
      ASSERT(ava_pos.size(), (CONSOLE::PLC_ERROR %
                              "NOT enough resource to place this design."));

      rand_idx = rand() % ava_pos.size();
      site = &_fpga.at(ava_pos[rand_idx]).site(site_type);
    }

    Point pos = ava_pos[rand_idx];
    pos.z = site->get_unocc_z_pos();
    site->_occ_insts[pos.z] = inst;
    ++site->_occ;

    inst->set_curr_logic_pos(pos);
    inst->set_curr_loc_site(site);
    inst->set_swapable_type(FLOORPLAN::SITE);

    SwapObject *obj = _nl_info.add_swap_object(FLOORPLAN::SITE);
    obj->fill(inst);
  }
  return (_nl_info.num_swap_objects() == 0);
}

bool Floorplan::find_init_pos_for_carrychain(int chain_length,
                                             std::vector<Point> &pos) {
  string DeviceName = FPGADesign::instance()->name();
  int BottomRow = 3;
  if (DeviceName == "FDP500K")
    BottomRow = 5;
  static const int ROW_OF_BOT_CENTER_TILE =
      _dev_info.device_scale().x - BottomRow;
  static const int rows = _dev_info.device_scale().x;
  static const int cols = _dev_info.device_scale().y;

  static int tile_col = cols / 2;
  static bool used_lower = cols % 2 ? false : true;

  bool found = false;
  vector<int> carry_chain1 =
      DEVICE::carry_chain[0]; // ����V2����ʼ������ʱ��λ��ֻ���ǲ�����0��1λ��

  Point chain_pos(ROW_OF_BOT_CENTER_TILE, tile_col, 0);
  while (tile_col >= 0 && tile_col < cols) {
    if (_fpga.at(chain_pos).exist_site(Site::SLICE)) {
      for (int row = chain_pos.x; row >= 0; --row) {
        Tile &tile = _fpga.at(row, tile_col);
        if (tile.exist_site(Site::SLICE)) {
          for (int zpos : carry_chain1) {
            if (!tile.site(Site::SLICE)._occ_insts[zpos]) {
              pos.push_back(Point(row, tile_col, zpos));
              if (pos.size() >= chain_length)
                break;
            } else {
              pos.clear();
              break; // ��λ������ʼ��ֻ����z=0��λ��
            }
          }
        }

        if (pos.size() >= chain_length) {
          found = true;
          break;
        }
      }
    }

    if (used_lower)
      tile_col = cols - tile_col - 1;
    else
      tile_col = cols - tile_col;
    used_lower = !used_lower;
    chain_pos.y = tile_col;

    if (found)
      return true;
    else
      pos.clear();
  }

  if (tile_col < 0 || tile_col >= cols) {
    tile_col = cols / 2;
    used_lower = cols % 2 ? false : true;
  }

  return false;
}

/************************************************************************/
/*	���ܣ��ҵ�һ���ɽ����ĵ�Ԫsite
 *	������
                from_obj:	Ҫ�����ĵ�Ԫ
                pos_to:		Ŀ�ĵ�ַ
                rlim:		����Լ��
 *	����ֵ��Result : pair<bool, FLOORPLAN::SwapableType>��˵������һ�ֿɽ�������
 *	˵����
 *		�����_available_pos���ҵ�һ���ɽ�����λ�ã����������rlim����ô�ͽ���
 *
 ��_available_pos�����޳�������_available_posҲ���𲽱�С
 *
 *		�޳��ı�׼��
 */
/************************************************************************/
Floorplan::Result Floorplan::satisfy_cst_rules_for_site(SwapObject *from_obj,
                                                        Point &pos_to,
                                                        double rlim) {
  // �õ�form obj��to obj����Ϣ
  Site &site_from = from_obj->base_loc_site();
  int rand_to_idx = rand() % site_from._available_pos.size();
  pos_to = site_from._available_pos[rand_to_idx];
  Site &site_to = _fpga.at(pos_to).site(site_from._type);

  // if the site type is macro ,we don't accept this swap
  // �õ�to obj�ĺ�����
  FLOORPLAN::SwapableType site_to_macro_type = site_to.macro_type();
  // ���from��carry chain��to��LUT6����ô����
  if (site_from.macro_type() == FLOORPLAN::CARRY_CHAIN &&
      site_to_macro_type == FLOORPLAN::LUT6)
    return make_pair(false, FLOORPLAN::IGNORE);
  // ���from��fix����to��LUT6
  if (site_from.has_fixed_insts() && site_to_macro_type == FLOORPLAN::LUT6)
    return make_pair(false, FLOORPLAN::IGNORE);
  // �õ���ַ��Ϣ
  Point pos_from = site_from._logic_pos;
  Point phy_pos_from = site_from._owner->phy_pos();
  Point phy_pos_to = site_to._owner->phy_pos();
  // �ж��Ƿ���rlim��Χ�ڣ�����to��from���غ�

  // optimization for IOB and BLOCKRAM
  if (site_from._type == Site::IOB || site_from._type == Site::BLOCKRAM)
    rlim = max_rlim();

  if (abs(phy_pos_from.x - phy_pos_to.x) <= rlim &&
      abs(phy_pos_from.y - phy_pos_to.y) <= rlim &&
      (pos_from.x != pos_to.x || pos_from.y != pos_to.y)) {
    int num_fixed_insts = 0, num_carrychain_insts = 0;
    // ��ÿһ��to obj�����fix��carry chainռ�õ���Ŀ
    for (PLCInstance *inst : site_to._occ_insts) {
      if (inst == nullptr)
        continue;
      // �����fix
      if (inst->is_fixed())
        ++num_fixed_insts;
      // ���������carry chain
      else if (inst->swapable_type() == FLOORPLAN::CARRY_CHAIN)
        ++num_carrychain_insts;
    }
    // ���fix��Ŀ��carry chain����ĿС��������˵�����п��Խ�����
    if (num_fixed_insts + num_carrychain_insts < site_to._capacity)
      return make_pair(true, site_to_macro_type);
    // ���fix����Ŀ���ڵ�������˵��û�п��Խ�����
    else if (num_fixed_insts >= site_to._capacity)
      site_from.pop_unavailable_pos(rand_to_idx);
    // ���carry chain����Ŀ���ڵ��������Ļ�ԭ��Ϊ����pop��
    // ��Ϊ���ǵ�carry chain�������ߺ��λ�þͿ����ڽ�����
    // �������ֻᵱavailable_posֻ��һ��λ�ò��Ҹ�λ�þ���carry chainռ�ݺ�
    // ����pop�ᵼ����ѭ�������Ի���Ӧ��pop
    else if (num_carrychain_insts >= site_to._capacity)
      site_from.pop_unavailable_pos(rand_to_idx);
  } else {
    // ������ڷ�Χ�ڣ�pop��
    site_from.pop_unavailable_pos(rand_to_idx);
  }

  return make_pair(false, FLOORPLAN::IGNORE);
}
/************************************************************************/
/*	���ܣ��ҵ�һ���ɽ�����LUT6
 *	������
        from_obj:	Ҫ�����ĵ�Ԫ
        pos_to:		Ŀ�ĵ�ַ
        rlim:		����Լ��
 *	����ֵ��Result : pair<bool, FLOORPLAN::SwapableType>��˵������һ�ֿɽ�������
 *	˵����
 *		�����_available_pos���ҵ�һ���ɽ�����λ�ã����������rlim����ô�ͽ���
 *
 ��_available_pos�����޳�������_available_posҲ���𲽱�С
 *
 *		�޳��ı�׼��
 */
/************************************************************************/
Floorplan::Result Floorplan::satisfy_cst_rules_for_lut6(SwapObject *from_obj,
                                                        Point &pos_to,
                                                        double rlim) {
  // ���ѡȡһ��to position
  Site &site_from = from_obj->base_loc_site();
  int rand_to_idx = rand() % site_from._available_pos.size();
  pos_to = site_from._available_pos[rand_to_idx];
  Site &site_to = _fpga.at(pos_to).site(site_from._type);
  // ���site_to��fix����ô����
  // ���������⣺���ȫ����fixed����ôѭ���˲���
  if (site_to.has_fixed_insts())
    return make_pair(false, FLOORPLAN::IGNORE);
  // ���to obj�ĺ�����Ϊcarry chain,����
  FLOORPLAN::SwapableType site_to_macro_type = site_to.macro_type();
  if (site_to_macro_type == FLOORPLAN::CARRY_CHAIN) {
    // if not pop here, it will cause infinite loop in some cases
    site_from.pop_unavailable_pos(rand_to_idx);
    return make_pair(false, FLOORPLAN::IGNORE);
  }
  // �õ�from��to�ĵ�ַ��Ϣ
  Point pos_from = site_from._logic_pos;
  Point phy_pos_from = site_from._owner->phy_pos();
  Point phy_pos_to = site_to._owner->phy_pos();
  // ����rlimԼ��
  if (abs(phy_pos_from.x - phy_pos_to.x) <= rlim &&
      abs(phy_pos_from.y - phy_pos_to.y) <= rlim &&
      (pos_from.x != pos_to.x || pos_from.y != pos_to.y)) {
    return make_pair(true, site_to_macro_type);
  } else {
    // �����㣬�޳�
    site_from.pop_unavailable_pos(rand_to_idx);
    return make_pair(false, FLOORPLAN::IGNORE);
  }
}
/************************************************************************/
/*	���ܣ��ҵ�һ���ɽ�����carry chain
 *	������
                from_obj:	Ҫ�����ĵ�Ԫ
                pos_to:		Ŀ�ĵ�ַ
                rlim:		����Լ��
 *	����ֵ��Result : pair<bool, FLOORPLAN::SwapableType>��˵������һ�ֿɽ�������
 *	˵����
 *		�����_available_pos���ҵ�һ���ɽ�����λ�ã����������rlim����ô�ͽ���
 *
 ��_available_pos�����޳�������_available_posҲ���𲽱�С
 *
 *		�޳��ı�׼��
 */
/************************************************************************/
Floorplan::Result
Floorplan::satisfy_cst_rules_for_carrychain(SwapObject *from_obj, Point &pos_to,
                                            double rlim) {
  // 		Site& site_from   = from_obj->base_loc_site();
  // 		int carry_size = DEVICE::carry_chain[0].size();
  // 		//������������飬�ڸ���λ��ר�õ�qualified_pos���ҿ��õ�����
  // 		//����site��������ȣ������ҳ���������϶������õģ����ǳ�����rlim�ķ�Χ��
  // 		SwapObject::QualifiedPos& qualified_pos =
  // from_obj->get_qualified_pos(); 		int rand_index_of_y =
  // rand()%qualified_pos.size();
  // SwapObject::QualifiedPos::iterator it = qualified_pos.begin();
  // for(int i = 0; i < rand_index_of_y; i++) 			it++;
  // int y = it -> first; 		int rand_index_of_carry =
  // rand()%DEVICE::carry_chain.size(); 		int z = rand_index_of_carry *
  // carry_size; 		int rand_index_of_x
  // =rand()%qualified_pos[y][rand_index_of_carry].size(); 		int x =
  // qualified_pos[y][rand_index_of_carry][rand_index_of_x];
  //
  // 		pos_to.x = x;
  // 		pos_to.y = y;
  // 		pos_to.z = z;
  //
  // 		Site& site_to      = _fpga.at(pos_to).site(site_from._type);
  // 		Point pos_from	   = site_from._logic_pos;
  // 		Point phy_pos_from = site_from._owner->phy_pos();
  // 		Point phy_pos_to   = site_to._owner->phy_pos();
  //
  // 		if ( abs(phy_pos_from.x - phy_pos_to.x) <= rlim &&
  // 			 abs(phy_pos_from.y - phy_pos_to.y) <= rlim &&
  // 			(pos_from.x != pos_to.x || pos_from.y != pos_to.y) )
  // 		{
  // 			return make_pair(true, FLOORPLAN::SITE);
  // 		} else {
  // 			//���������rlim�ķ�Χ��pop����Ӧ�����꣬�����x�����Ļ���ֻɾ�����x����
  // 			if(abs(phy_pos_from.x - phy_pos_to.x) > rlim)
  // 			{
  // 				qualified_pos[y][rand_index_of_carry][rand_index_of_x]
  // =qualified_pos[y][rand_index_of_carry].back();
  // qualified_pos[y].pop_back();
  // 			}
  // 			//�����y����rlim��Χ��pop�����е�����
  // 			if(abs(phy_pos_from.y - phy_pos_to.y) > rlim)
  // 				qualified_pos.erase(y);
  //
  // 			return make_pair(false, FLOORPLAN::IGNORE);
  // 		}
  Site &site_from = from_obj->base_loc_site();
  int rand_to_idx = rand() % site_from._available_pos.size();
  pos_to = site_from._available_pos[rand_to_idx];

  bool is_qualified = false;
  for (int index = 0; index < DEVICE::carry_chain.size(); index++) {
    if (from_obj->exist_qualified_pos(_dev_info.device_scale().x, pos_to,
                                      index)) {
      is_qualified = true;
      pos_to.z = index * DEVICE::carry_chain[0].size();
      break;
    }
  }
  if (!is_qualified) {
    // if not pop here, it will cause infinite loop in some cases
    site_from.pop_unavailable_pos(rand_to_idx);
    return make_pair(false, FLOORPLAN::IGNORE);
  }

  Site &site_to = _fpga.at(pos_to).site(site_from._type);
  Point pos_from = site_from._logic_pos;
  Point phy_pos_from = site_from._owner->phy_pos();
  Point phy_pos_to = site_to._owner->phy_pos();

  if (abs(phy_pos_from.x - phy_pos_to.x) <= rlim &&
      abs(phy_pos_from.y - phy_pos_to.y) <= rlim &&
      (pos_from.x != pos_to.x || pos_from.y != pos_to.y)) {
    return make_pair(true, FLOORPLAN::SITE);
  } else {
    site_from.pop_unavailable_pos(rand_to_idx);
    return make_pair(false, FLOORPLAN::IGNORE);
  }
}
/************************************************************************/
/*	���ܣ������ƶ��е�z�����һ�е�qualified_posֵ
 *	������
 *		tile_col:int,tile�е�������
 *		z_pos:int,���е�z����
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
void Floorplan::update_qualified_pos_for_carrychain(int tile_col, int index) {
  static int num_rows = _fpga.size().x - 1;
  vector<int> carry_chain_index = DEVICE::carry_chain[index];
  size_t carry_size = DEVICE::carry_chain[0].size();

  for (NLInfo::MacroInfo::CarryChains::value_type &chain :
       _nl_info.carry_chains()) {
    for (SwapObject *obj : chain.second)
      obj->clr_qualified_pos(tile_col, index);
  }

  int length = 0;
  vector<Point> consecutive_pos_vec;
  for (int tile_row = num_rows; tile_row >= 0; --tile_row) {
    Tile &tile = _fpga.at(tile_row, tile_col);
    if (tile.exist_site(Site::SLICE)) {
      Site &slice_site = tile.site(Site::SLICE);
      bool continue_flag = true;
      for (int i = 0; i < carry_chain_index.size(); i++) {
        int zpos = carry_chain_index[i];
        PLCInstance *slice_inst = slice_site._occ_insts[zpos];
        if (!slice_inst ||
            (!slice_inst->is_macro() && !slice_inst->is_fixed())) {
          ++length;
          if (i == 0) // ֻ��tile�н�λ���Ŀ�ʼλ����Ϊ��λ������ʼ��
            consecutive_pos_vec.push_back(Point(tile_row, tile_col, zpos));
        } else {
          continue_flag =
              false; // �����һ��tile�еĽ�λ�������˱�ռ�е�inst��������������Ա��ж�
          break;
        }
      }
      if (continue_flag)
        continue;
    } else if (tile_row > 0)
      continue;

    // ����carry chain�Ŀ���λ��
    if (length != 0) {
      for (NLInfo::MacroInfo::CarryChains::value_type &chain :
           _nl_info.carry_chains()) {
        if (chain.first <= length) {
          int index_for_pos = consecutive_pos_vec.size();
          // �������whileѭ�������ڴ洢��������vector���ܷ��½�λ�������һ�������������
          // ��Ϊconsecutive_pos_vec��ֻ����ÿ��tile�н�λ����ʼ�����꣬���Լ��㷽���Ƚ����
          while ((length - (index_for_pos - 1) * carry_size) < chain.first)
            index_for_pos--;
          for (SwapObject *obj : chain.second) {
            for (int i = 0; i < index_for_pos; ++i)
              obj->add_qualified_pos(index, consecutive_pos_vec[i]);
          }
        }
      }
      length = 0;
      consecutive_pos_vec.clear();
    }
  } // end of "for"
}

// 	int Floorplan::index_of_carry_chain(int zpos)
// 	{
// 		bool found = false;
// 		int index = 0;
// 		for (int i = 0; i < DEVICE::carry_chain.size(); i++)
// 		{
// 			vector<int> one_carry_chain = DEVICE::carry_chain[i]
// 			for (int z_pos_of_carry: one_carry_chain)
// 			{
// 				if (z_pos_of_carry == zpos)
// 				{
// 					found = true;
// 					break;
// 				}
// 			}
// 			if (found)
// 			{
// 				index = i;
// 				break;
// 			}
// 		}
// 		ASSERT(found,CONSOLE::PLC_ERROR % (zpos + ": don't have a carry
// chain.")) 		return index;
// 	}

void Floorplan::update_qualified_pos_for_carrychain(SwapInsts &from_insts,
                                                    SwapInsts &to_insts) {
  typedef set<pair<int, int>> AffectedZCols;
  AffectedZCols affected_z_cols;

  PLCInstance *inst = nullptr;
  Point pos;
  int index;
  int carry_size = DEVICE::carry_chain[0].size(); // ÿ����λ����ռ��slice����
  for (int i = 0; i < from_insts.size(); ++i) {
    inst = from_insts[i];
    if (inst &&
        lexical_cast<Site::SiteType>(inst->module_type()) == Site::SLICE) {
      pos = inst->past_logic_pos();
      // index = index_of_carry_chain(pos.z);
      index = pos.z / carry_size;
      affected_z_cols.insert(make_pair(pos.y, index));
      pos = inst->curr_logic_pos();
      // index = index_of_carry_chain(pos.z);
      index = pos.z / carry_size;
      affected_z_cols.insert(make_pair(pos.y, index));
    }
    inst = to_insts[i];
    if (inst &&
        lexical_cast<Site::SiteType>(inst->module_type()) == Site::SLICE) {
      pos = inst->past_logic_pos();
      // index = index_of_carry_chain(pos.z);
      index = pos.z / carry_size;
      affected_z_cols.insert(make_pair(pos.y, index));
      pos = inst->curr_logic_pos();
      // index = index_of_carry_chain(pos.z);
      index = pos.z / carry_size;
      affected_z_cols.insert(make_pair(pos.y, index));
    }
  }
  // ����ÿһ����Ӱ����н��и���
  for (const AffectedZCols::value_type &z_col : affected_z_cols)
    update_qualified_pos_for_carrychain(z_col.first, z_col.second);
}

void Floorplan::swap_insts_for_site(SwapObject *from_obj, Point &pos_to,
                                    SwapInsts &from_insts, SwapInsts &to_insts,
                                    FLOORPLAN::SwapableType to_type) {
  Site &site_from = from_obj->base_loc_site();
  Site &site_to = _fpga.at(pos_to).site(site_from._type);

  switch (to_type) {
  // only 1 site will be swapped
  case FLOORPLAN::SITE:
    do {
      int rand_to_index = 0;
      rand_to_index = rand() % site_to._capacity;

      // move to
      if (rand_to_index >= site_to._occ) {
        pos_to.z = site_to.get_unocc_z_pos();
        ASSERT(pos_to.z != -1,
               (CONSOLE::PLC_ERROR % "can NOT found unoccupation z pos."));

        from_obj->update_place_info(0, pos_to, site_to);
        from_insts.push_back(from_obj->plc_insts()[0]);
        to_insts.push_back(nullptr);
      } else { // swap with
        PLCInstance *to_inst = site_to.get_occ_inst(rand_to_index);
        if (!to_inst)
          continue;
        pos_to = to_inst->curr_logic_pos();

        to_inst->set_curr_logic_pos(from_obj->base_logic_pos());
        to_inst->set_curr_loc_site(&site_from);
        from_obj->update_place_info(0, pos_to, site_to);
        from_insts.push_back(from_obj->plc_insts()[0]);
        to_insts.push_back(to_inst);
      }
      break;
    } while (true);
    break;
  // 2 slices will be swapped
  case FLOORPLAN::LUT6:
    pos_to.z = 0;
    for (int z = 0; z < DEVICE::NUM_SLICE_PER_TILE; ++z) {
      PLCInstance *from_inst = site_from._occ_insts[z];
      PLCInstance *to_inst = site_to._occ_insts[z];
      if (from_inst) {
        from_inst->set_curr_logic_pos(Point(pos_to.x, pos_to.y, z));
        from_inst->set_curr_loc_site(&site_to);
      }
      if (to_inst) {
        to_inst->set_curr_logic_pos(
            Point(site_from._logic_pos.x, site_from._logic_pos.y, z));
        to_inst->set_curr_loc_site(&site_from);
      }
      from_insts.push_back(from_inst);
      to_insts.push_back(to_inst);
    }
    break;
  // only 1 slice will be swapped, meanwhile the to_slice is NOT part of carry
  // chain
  case FLOORPLAN::CARRY_CHAIN:
    for (int z = 0; z < DEVICE::NUM_SLICE_PER_TILE; ++z) {
      PLCInstance *to_inst = site_to._occ_insts[z];
      if (!to_inst || to_inst->swapable_type() != FLOORPLAN::CARRY_CHAIN) {
        if (to_inst) {
          to_inst->set_curr_logic_pos(from_obj->base_logic_pos());
          to_inst->set_curr_loc_site(&site_from);
        }
        pos_to.z = z;
        from_obj->update_place_info(0, pos_to, site_to);
        from_insts.push_back(from_obj->plc_insts()[0]);
        to_insts.push_back(to_inst);
        break;
      }
    }
    break;
  default:
    ASSERT(0, (CONSOLE::PLC_ERROR % "unknown swap object."));
  }
}

void Floorplan::swap_insts_for_lut6(SwapObject *from_obj, Point &pos_to,
                                    SwapInsts &from_insts, SwapInsts &to_insts,
                                    FLOORPLAN::SwapableType to_type) {
  Point pos_from = from_obj->base_logic_pos();

  Site &site_from = from_obj->base_loc_site();
  Site &site_to = _fpga.at(pos_to).site(Site::SLICE);

  for (int z = 0; z < DEVICE::NUM_SLICE_PER_TILE; ++z) {
    PLCInstance *from_inst = site_from._occ_insts[z];
    PLCInstance *to_inst = site_to._occ_insts[z];

    from_inst->set_curr_logic_pos(Point(pos_to.x, pos_to.y, z));
    from_inst->set_curr_loc_site(&site_to);
    if (to_inst) {
      to_inst->set_curr_logic_pos(Point(pos_from.x, pos_from.y, z));
      to_inst->set_curr_loc_site(&site_from);
    }
    from_insts.push_back(from_inst);
    to_insts.push_back(to_inst);
  }
}

void Floorplan::swap_insts_for_carrychain(SwapObject *from_obj, Point &pos_to,
                                          SwapInsts &from_insts,
                                          SwapInsts &to_insts,
                                          FLOORPLAN::SwapableType to_type) {
  int count = 0, length = from_obj->plc_insts().size();
  vector<Point> chain_to_pos_vec;
  Point cur_pos = pos_to;
  while (count < length) {
    if (_fpga.at(cur_pos).exist_site(Site::SLICE)) {
      for (int i = 0; i < DEVICE::carry_chain[0].size(); i++) {
        ++count;
        Point tmp(cur_pos.x, cur_pos.y, cur_pos.z + i);
        chain_to_pos_vec.push_back(tmp);
        if (count == length)
          break;
      }
    }
    --cur_pos.x;
  }
  ASSERT(from_obj->plc_insts().size() == chain_to_pos_vec.size(),
         (CONSOLE::PLC_ERROR % "illegal chain pos for swap."));

  for (int i = 0; i < length; ++i) {
    PLCInstance *from_inst = from_obj->plc_insts()[i];

    Point pos_from = from_inst->curr_logic_pos();
    Point pos_to = chain_to_pos_vec[i];

    Site &site_from = *(from_inst->curr_loc_site());
    Site &site_to = _fpga.at(pos_to).site(Site::SLICE);

    PLCInstance *to_inst = site_to._occ_insts[pos_to.z];

    from_inst->set_curr_logic_pos(pos_to);
    from_inst->set_curr_loc_site(&site_to);
    if (to_inst) {
      to_inst->set_curr_logic_pos(pos_from);
      to_inst->set_curr_loc_site(&site_from);
    }
    from_insts.push_back(from_inst);
    to_insts.push_back(to_inst);
  }
}

/************************************************************************/
/*	���ܣ��ҵ�һ���ɽ����ĵ�Ԫ
*	������
                from_obj:	Ҫ�����ĵ�Ԫ
                pos_to:		Ŀ�ĵ�ַ
                rlim:		����Լ��
*	����ֵ��SwapableType��˵������һ�ֿɽ�������
*	˵����
*		ÿ��site�����_available_pos��������洢������оƬ��Χ�ɽ�����λ�ã�
*
����SA�Ĺ��̣�rlim�𽥱�С����ɽ�����Χ����С
*		��ִ��find_to�����е���satisfy_cst_rules_for_XXXXʱ��
*		�����_available_pos���ҵ�һ���ɽ�����λ�ã����������rlim����ô�ͽ���
*
��_available_pos�����޳�������_available_posҲ���𲽱�С
*/
/************************************************************************/
FLOORPLAN::SwapableType Floorplan::find_to(SwapObject *from_obj, Point &pos_to,
                                           double rlim) {
  Site &site_from = from_obj->base_loc_site();
  do {
    // ���û�пɽ���λ�ã�����
    if (site_from._available_pos.size() == 0)
      return FLOORPLAN::IGNORE;
    // Ѱ��һ���ɽ���λ��
    bool has_found = false;
    FLOORPLAN::SwapableType to_type = FLOORPLAN::IGNORE;
    switch (from_obj->type()) {
    case FLOORPLAN::SITE:
      boost::tie(has_found, to_type) =
          satisfy_cst_rules_for_site(from_obj, pos_to, rlim);
      break;
    case FLOORPLAN::LUT6:
      boost::tie(has_found, to_type) =
          satisfy_cst_rules_for_lut6(from_obj, pos_to, rlim);
      break;
    case FLOORPLAN::CARRY_CHAIN:
      boost::tie(has_found, to_type) =
          satisfy_cst_rules_for_carrychain(from_obj, pos_to, rlim);
      break;
    default:
      ASSERT(0, (CONSOLE::PLC_ERROR % "unknown swap object."));
    }
    // �ҵ�������
    if (has_found)
      return to_type;

  } while (true);
}
/************************************************************************/
/*	���ܣ�����������Ԫinstance
*	������
                from_insts:	Դ������Ԫ
                to_insts:	Ŀ�Ľ�����Ԫ
                pos_from:	Դλ��
                pos_to:		Ŀ�ĵ�ַ
                rlim:		����Լ��
*	����ֵ��SwapableType��˵������һ�ֿɽ�������
*	˵���������ѡһ��From obj��Ȼ��Ѱ��һ��to obj�������н���
*/
/************************************************************************/
void Floorplan::swap_insts(SwapInsts &from_insts, SwapInsts &to_insts,
                           Point &pos_from, Point &pos_to, double rlim) {
  FLOORPLAN::SwapableType to_type;
  SwapObject *from_obj = nullptr;
  // �������_nl_info.num_swap_objects()Ϊ0����ô�Ͳ�������ѭ���������Ժ���Ҫ�޸�
  do {
    // ����ҵ�һ�����Խ�����from obj������Ǵ�������Ϣ����Ѱ��
    int rand_from_idx = rand() % _nl_info.num_swap_objects();
    from_obj = _nl_info.swap_object(rand_from_idx);
    // �ҵ�һ���ɽ���to obj
    to_type = find_to(from_obj, pos_to, rlim);
    // ���û���ҵ�һ���ɽ��� to
    // obj����ô��swap_obj�����޳����from obj
    if (to_type == FLOORPLAN::IGNORE)
      _nl_info.pop_swap_object(rand_from_idx);

  } while (to_type == FLOORPLAN::IGNORE);
  // ������Ӧ��isntance
  switch (from_obj->type()) {
  case FLOORPLAN::SITE:
    swap_insts_for_site(from_obj, pos_to, from_insts, to_insts, to_type);
    break;
  case FLOORPLAN::LUT6:
    swap_insts_for_lut6(from_obj, pos_to, from_insts, to_insts, to_type);
    break;
  case FLOORPLAN::CARRY_CHAIN:
    swap_insts_for_carrychain(from_obj, pos_to, from_insts, to_insts, to_type);
    break;
  default:
    throw "Not handled switch case";
  }
}
/************************************************************************/
/*	���ܣ�try swap�Ժ�����Ƿ��ܽ����������ǽ������ǻ��˲���
 *	������
 *		keey_swap: bool,�жϽ����Ƿ񱻽���
 *		from_insts:SwapInsts,from obj
 *		to_insts:SwapInsts,to obj
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
void Floorplan::maintain(bool keey_swap, SwapInsts &from_insts,
                         SwapInsts &to_insts) {
  // �ж��������������Ƿ�����ͬ��С
  ASSERT(from_insts.size() == to_insts.size(),
         (CONSOLE::PLC_ERROR % "swap instances' size not equal."));

  int size = from_insts.size();
  // ����ÿһ������inst
  for (int i = 0; i < size; ++i) {
    PLCInstance *from_inst = from_insts[i];
    PLCInstance *to_inst = to_insts[i];

    Point pos_from, pos_to;
    Site::SiteType site_type;
    // �Ѿ���������
    if (from_inst) {
      pos_from = from_inst->past_logic_pos();
      pos_to = from_inst->curr_logic_pos();
      site_type = from_inst->curr_loc_site()->_type;
    } else if (to_inst) {
      pos_from = to_inst->curr_logic_pos();
      pos_to = to_inst->past_logic_pos();
      site_type = to_inst->curr_loc_site()->_type;
    } else
      throw(CONSOLE::PLC_ERROR % "both from_inst & to_inst are nullptr.");
    // �õ�site from��site to
    Site &site_from = _fpga.at(pos_from).site(site_type);
    Site &site_to = _fpga.at(pos_to).site(site_type);
    // �������������
    if (keey_swap) {
      //
      if (_mode == PLACER::TIMING_DRIVEN) {
        update_tcost(from_inst, to_inst);
        update_tcost(to_inst, from_inst);
      }
      // ����site_from��site_to
      site_from._occ_insts[pos_from.z] = to_inst;
      site_to._occ_insts[pos_to.z] = from_inst;
      // ���from�е���toû�У���ô����
      if (from_inst && !to_inst) {
        --site_from._occ;
        ++site_to._occ;
      }
      // ���fromû�У�����to��,����
      else if (!from_inst && to_inst) {
        ++site_from._occ;
        --site_to._occ;
      }
      //				else if(from_inst && to_inst) { /* need
      // to do nothing */ }
    }
    // swap was rejected,reset all information
    else {
      // �ָ�from obj
      if (from_inst) {
        from_inst->reset_logic_pos();
        from_inst->reset_loc_site();
      }
      // �ָ�to obj
      if (to_inst) {
        to_inst->reset_logic_pos();
        to_inst->reset_loc_site();
      }
    }
  } // end of "for"
  // �������Ĵ���
  typedef pair<PLCInstance *, PLCNet *> AffectedNet;
  if (keey_swap) {
    // ����������ܣ����������pre_bb_cost��affected type
    for (AffectedNet &affected_net : _nl_info.affected_nets()) {
      affected_net.second->reset_pre_bb_cost();
      affected_net.second->reset_affected_type();
    }
  } else {
    // �������û�б�����
    for (AffectedNet &affected_net : _nl_info.affected_nets()) {
      affected_net.second->restore_bounding_box();
      affected_net.second->reset_affected_type();
    }
  }
  // ��������������¼�����Ӱ����е�_qualified_pos
  if (keey_swap)
    update_qualified_pos_for_carrychain(from_insts, to_insts);
}
/************************************************************************/
/*	���ܣ�����һ��isnts����Ӱ������
 *	������
 *		insts:SwapInsts,��Ӱ���swap insts
 *		t:AffectedType,Ӱ������������
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
void Floorplan::find_affected_nets(SwapInsts &insts, PLCNet::AffectedType t) {
  for (PLCInstance *inst : insts) {
    if (inst) {
      for (Pin *pin : inst->pins()) {
        PLCNet *p_net = static_cast<PLCNet *>(pin->net());
        if (p_net && !p_net->is_ignored()) {
          // ������ǰ�Ǵ��ģ�����ע��
          // p_net->set_affected_type(PLCNet::FROM);
          p_net->set_affected_type(t);
          // ��ֹ�ظ����ӣ����<0��˵��û�м��������
          if (p_net->pre_bb_cost() < 0.) {
            _nl_info.add_affected_net(inst, p_net);
            p_net->set_pre_bb_cost(1.0);
          }
        }
      }
    }
  }
}

/************************************************************************/
/*	���ܣ�����һ�ν���������insts����Ӱ������
 *	������
 *		from_insts:SwapInsts,��Ӱ���swap insts
 *		to_insts:SwapInsts,��Ӱ���swap insts
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
void Floorplan::find_affected_nets(SwapInsts &from_insts, SwapInsts &to_insts) {
  // �Ƚ���Ӱ���������
  _nl_info.clr_affected_nets();
  // �ֱ����FROM��TO����Ӱ������
  find_affected_nets(from_insts, PLCNet::FROM);
  find_affected_nets(to_insts, PLCNet::TO);
}

double Floorplan::compute_bb_cost() {
  double cost = 0.;
  for (PLCNet *net :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->nets()) {
    if (net->is_ignored())
      continue;

    net->compute_bounding_box(_dev_info.device_scale());
    cost += net->compute_bb_cost(_dev_info.inv_of_chan_width());
  }

  return cost;
}

double Floorplan::compute_tcost(TEngine::WORK_MODE emode, double crit_exp) {
  double dmax;
  double tcost = 0.;
  PLCInstance *source_inst, *sink_inst;
  Property<COS::TData> &delays =
      create_temp_property<COS::TData>(COS::PIN, PIN::DELAY);
  // if(dynamic_cast<TDesign*>(_nl_info.design()) != nullptr )
  dmax = _nl_info.design()->timing_analyse(emode);

  for (PLCNet *net :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->nets()) {
    if (net->is_ignored())
      continue;

    source_inst =
        static_cast<PLCInstance *>(net->source_pins().begin()->owner());
    for (Pin *pin : net->sink_pins()) {
      sink_inst = static_cast<PLCInstance *>(pin->owner());

      double delay = _dev_info.get_p2p_delay(source_inst, sink_inst);
      pin->set_property(delays, TData(delay, delay));
    }

    tcost += net->compute_tcost(dmax, crit_exp);
  }

  return tcost;
}

double Floorplan::compute_delta_bb_cost(SwapInsts &from_insts,
                                        SwapInsts &to_insts) {
  find_affected_nets(from_insts, to_insts);

  double delta_bb_cost = 0.;
  typedef pair<PLCInstance *, PLCNet *> AffectNet;
  for (const AffectNet &net : _nl_info.affected_nets()) {
    PLCInstance *cause_inst = net.first;
    PLCNet *affected_net = net.second;

    affected_net->save_bounding_box();

    if ((affected_net->affected_type() & PLCNet::FROM) &&
        (affected_net->affected_type() & PLCNet::TO))
      continue;

    if (affected_net->affected_type() & PLCNet::FROM)
      affected_net->update_bounding_box(cause_inst->past_logic_pos(),
                                        cause_inst->curr_logic_pos(),
                                        _dev_info.device_scale());
    else if (affected_net->affected_type() & PLCNet::TO)
      affected_net->update_bounding_box(cause_inst->past_logic_pos(),
                                        cause_inst->curr_logic_pos(),
                                        _dev_info.device_scale());

    affected_net->compute_bb_cost(_dev_info.inv_of_chan_width());

    delta_bb_cost += affected_net->bb_cost() - affected_net->pre_bb_cost();
  }

  return delta_bb_cost;
}

double Floorplan::compute_delta_tcost(PLCInstance *target,
                                      PLCInstance *reference) {
  double delta_delay = 0., delta_tcost = 0.;
  Property<COS::TData> &delays =
      create_temp_property<COS::TData>(COS::PIN, PIN::DELAY);
  Property<double> &tcosts = create_temp_property<double>(COS::PIN, PIN::TCOST);
  Property<double> &temp_delays =
      create_temp_property<double>(COS::PIN, PIN::TEMP_DELAY);
  Property<double> &temp_tcost =
      create_temp_property<double>(COS::PIN, PIN::TEMP_TCOST);
  Property<double> &crits = create_temp_property<double>(COS::PIN, PIN::CRIT);

  if (!target)
    return delta_tcost;

  for (Pin *pin : target->pins()) {
    PLCNet *p_net = static_cast<PLCNet *>(pin->net());
    if (!p_net || p_net->is_ignored())
      continue;

    if (pin->is_sink()) {
      PLCInstance *p_inst =
          static_cast<PLCInstance *>(p_net->source_pins().begin()->owner());
      if (p_inst != target && p_inst != reference) {
        double delay = _dev_info.get_p2p_delay(p_inst, target);
        double tcost = pin->property_value(crits) * delay;
        pin->set_property(temp_delays, delay);
        pin->set_property(temp_tcost, tcost);

        delta_delay += delay - pin->property_value(delays)._rising;
        delta_tcost += tcost - pin->property_value(tcosts);
      }
    } else { // pin is source
      for (Pin *sink_pin : p_net->sink_pins()) {
        PLCInstance *p_inst = static_cast<PLCInstance *>(sink_pin->owner());
        double delay = _dev_info.get_p2p_delay(target, p_inst);
        double tcost = sink_pin->property_value(crits) * delay;
        sink_pin->set_property(temp_delays, delay);
        sink_pin->set_property(temp_tcost, tcost);

        delta_delay += delay - sink_pin->property_value(delays)._rising;
        delta_tcost += tcost - sink_pin->property_value(tcosts);
      }
    }
  }

  return delta_tcost;
}

double Floorplan::compute_delta_tcost(SwapInsts &from_insts,
                                      SwapInsts &to_insts) {
  double delta_tcost = 0.;

  for (int i = 0; i < from_insts.size(); ++i) {
    delta_tcost += compute_delta_tcost(from_insts[i], to_insts[i]);
    delta_tcost += compute_delta_tcost(to_insts[i], from_insts[i]);
  }

  return delta_tcost;
}

pair<double, double> Floorplan::recompute_cost() {
  double new_bb_cost = 0., new_tcost = 0.;
  for (PLCNet *net :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->nets()) {
    if (net->is_ignored())
      continue;

    new_bb_cost += net->bb_cost();
    if (_mode == PLACER::TIMING_DRIVEN)
      new_tcost += net->get_total_tcost();
  }

  return make_pair(new_bb_cost, new_tcost);
}

void Floorplan::update_tcost(PLCInstance *target, PLCInstance *reference) {
  Property<COS::TData> &delays =
      create_temp_property<COS::TData>(COS::PIN, PIN::DELAY);
  Property<double> &tcosts = create_temp_property<double>(COS::PIN, PIN::TCOST);
  Property<double> &temp_delays =
      create_temp_property<double>(COS::PIN, PIN::TEMP_DELAY);
  Property<double> &temp_tcost =
      create_temp_property<double>(COS::PIN, PIN::TEMP_TCOST);

  if (!target)
    return;

  for (Pin *pin : target->pins()) {
    PLCNet *p_net = static_cast<PLCNet *>(pin->net());
    if (!p_net || p_net->is_ignored())
      continue;

    if (pin->is_sink()) {
      PLCInstance *p_inst =
          static_cast<PLCInstance *>(p_net->source_pins().begin()->owner());
      if (p_inst != target && p_inst != reference) {
        double temp_value = pin->property_value(temp_delays);
        pin->set_property(delays, TData(temp_value, temp_value));

        temp_value = pin->property_value(temp_tcost);
        pin->set_property(tcosts, temp_value);
      }
    } else {
      for (Pin *sink_pin : p_net->sink_pins()) {
        double temp_value = sink_pin->property_value(temp_delays);
        sink_pin->set_property(delays, TData(temp_value, temp_value));

        temp_value = sink_pin->property_value(temp_tcost);
        sink_pin->set_property(tcosts, temp_value);
      }
    }
  }
}

/************************************************************************/
/*	���ܣ��趨��������е�һЩnetsΪignored
 *	������void
 *	����ֵ�� void
 *	˵����1. carry chain�������������Ϊignored
 *		  2. netֻ������û��pins(ISE����������������)
 *		  3. CONST_GENERATE������õ���Ŀǰ�汾������
 *		  4. GCLKIOB��net��Ϊignored
 *		  5. Port��CELL_pinҲ����Ϊignored��ͬʱ���cell_pin�Ƿ����ӣ�
 */
/************************************************************************/
void Floorplan::set_ignored_nets() {
  for (PLCNet *net :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->nets()) {
    if (net->type() == COS::CARRY || net->type() == COS::CLOCK ||
        net->type() == COS::VCC || !net->num_pins()
#ifndef CONST_GENERATE
        ||
        !count_if(net->pins(), [](const Pin *pin) { return pin->is_source(); })
#endif
    )
      net->set_ignored();
  }

  for (PLCInstance *inst :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->instances()) {
    Site::SiteType site_type =
        lexical_cast<Site::SiteType>(inst->module_type());
    if (site_type == Site::GCLKIOB) {
      for (Pin *pin : inst->pins())
        if (pin->net())
          static_cast<PLCNet *>(pin->net())->set_ignored();
    }
  }

  for (Pin *pin :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->pins()) {
    //			ASSERT(port.cell_pin().net(),
    //				   place_error(CONSOLE::PLC_ERROR % (port.name()
    //+ ": cell pin unconnect.")));
    // allow cell pin of top cell unconnect
    // if(pin->name()=="out[2]")
    //	int a=2;
    if (pin->net())
      static_cast<PLCNet *>(pin->net())->set_ignored();
  }
}

/************************************************************************/
/*	���ܣ����Լ���ļ�����ȷ��
 *	������void
 *	����ֵ�� void
 *	˵����1. carry chain�������������Ϊignored
 *		  2.
 *		  3.
 *		  4. GCLKIOB��net��Ϊignored
 *		  5. Port��CELL_pinҲ����Ϊignored��ͬʱ���cell_pin�Ƿ����ӣ�
 */
/************************************************************************/
void Floorplan::check_and_save_cst() {
  const int nx = _dev_info.device_scale().x;
  const int ny = _dev_info.device_scale().y;
  Property<string> &pads = create_temp_property<string>(COS::INSTANCE, "pad");
  Property<Point> &positions =
      create_property<Point>(COS::INSTANCE, INSTANCE::POSITION);
  Property<string> &SET_TYPE =
      create_property<string>(COS::INSTANCE, INSTANCE::SET_TYPE);

  Point invalid_pos;
  for (PLCInstance *inst :
       static_cast<PLCModule *>(_nl_info.design()->top_module())->instances()) {
    // ���Լ��λ�ò���ȷ������
    string position = inst->property_value(pads);
    // instû�б�Լ��
    if (position == "")
      continue;

    Point pos;
    try {
      pos = lexical_cast<Point>(position);
    } catch (boost::bad_lexical_cast &e) {
      pos = FPGADesign::instance()->find_pad_by_name(position);
    }
    ASSERT(pos != Point(),
           ("parse failed. constraint: illegal position for " + inst->name()));

    inst->set_property(positions, pos);
    Point logic_pos = inst->property_value(positions);
    // ���Լ��λ�ò����߼���Χ֮��
    ASSERT(logic_pos.x >= 0 && logic_pos.x <= nx && logic_pos.y >= 0 &&
               logic_pos.y <= ny && logic_pos.z >= 0,
           (CONSOLE::PLC_ERROR % (inst->name() + ": illegal constraint.")));
    // Լ�������Ͳ���FPGA��������
    Site::SiteType site_type =
        lexical_cast<Site::SiteType>(inst->module_type());

    ASSERT(site_type != Site::IGNORE,
           (CONSOLE::PLC_ERROR % (inst->name() + ": illegal cell type.")));
    // �ڸ�λ�����Ƿ����Լ����site����
    ASSERT(_fpga.at(logic_pos).exist_site(site_type),
           (CONSOLE::PLC_ERROR % (inst->name() + ": illgal constraint.")));
    // �����IOB����GCLKIOB����ô�ж��Ƿ��װ����
    if (site_type == Site::IOB || site_type == Site::GCLKIOB) {
      // is_io_bound���ж��Ƿ񱻷�װ����
      ASSERT(FPGADesign::instance()->is_io_bound(logic_pos),
             (CONSOLE::PLC_ERROR % (inst->name() + ": illegal constraint.")));
    }
    // ���FPGAλ��Ϊlogic_pos����Ϊsite_type��site
    Site &site = _fpga.at(logic_pos).site(site_type);
    // �ж�z�����Ƿ�����
    ASSERT(logic_pos.z < site._occ_insts.size(),
           (CONSOLE::PLC_ERROR % (inst->name() + ": illegal constraint.")));
    // �ж��Ƿ����ظ�Լ����ͬһλ��
    ASSERT(!site._occ_insts[logic_pos.z],
           (CONSOLE::PLC_ERROR % (inst->name() + ": illegal constraint.")));
    // ����Լ��carry chain
    // ASSERT(inst->property_value(SET_TYPE) != DEVICE::CARRY,
    //	   place_error(CONSOLE::PLC_ERROR % (inst->name() + ": illegal
    // constraint."))); ����Լ��LUT6
    ASSERT(inst->property_value(SET_TYPE) != DEVICE::LUT_6,
           (CONSOLE::PLC_ERROR % (inst->name() + ": illegal constraint.")));
    // ��instance����Լ����site����
    site._occ_insts[logic_pos.z] = inst;
    ++site._occ;
    // ����inst������
    inst->set_curr_logic_pos(logic_pos);
    inst->set_curr_loc_site(&site);
    inst->set_swapable_type(FLOORPLAN::SITE);
    inst->fix_inst();

    // ugly hard code, need optimization
    // GCLKIOB��Լ���жϣ�GCLK������ GCLK_pin �� GCLK buffer
    if (site_type == Site::GCLKIOB) {
      Pin *gclkout_pin = inst->pins().find(DEVICE::GCLKOUT);
      ASSERT(gclkout_pin && gclkout_pin->net(),
             (CONSOLE::PLC_ERROR % (inst->name() + ": NOT connect to GCLK.")));
      PLCInstance *gclkbuf = static_cast<PLCInstance *>(
          gclkout_pin->net()->sink_pins().begin()->owner());

      Point gclkbuf_pos = gclkbuf->property_value(positions);
      if (gclkbuf_pos == invalid_pos) {
        Site &gclk_site = _fpga.at(logic_pos).site(Site::GCLK);

        gclk_site._occ_insts[logic_pos.z] = gclkbuf;
        ++gclk_site._occ;

        gclkbuf->set_curr_logic_pos(logic_pos);
        gclkbuf->set_curr_loc_site(&gclk_site);
        gclkbuf->set_swapable_type(FLOORPLAN::SITE);
        gclkbuf->fix_inst();
      } else {
        ASSERT(
            gclkbuf_pos == logic_pos,
            (CONSOLE::PLC_ERROR %
             (gclkbuf->name() + ": pos not equal to GCLKIOB " + inst->name())));
      }
    }
  }
}

} // namespace Place
} // namespace FDU