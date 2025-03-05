#ifndef PLCFLOORPLAN_H
#define PLCFLOORPLAN_H

#include "plc_carry_infer.h"
#include "plc_factory.h"
#include "plc_lut6_infer.h"
#include "plc_netlist.h"
#include "tengine/tengine.hpp"

namespace FDU {
namespace Place {

class PLCInstance;
struct Site;
class Tile;
class SAPlacer;
class LUT6Inference;

/************************************************************************/
/*	功能：真正实现布局的类 */
/************************************************************************/
class Floorplan {
  friend class SAPlacer;

public:
  explicit Floorplan(TDesign *t) : _nl_info(t), _mode(PLACER::BOUNDING_BOX) {}
  virtual ~Floorplan() {}
  // 设置布局算法模式
  void set_place_mode(PLACER::PlaceMode m) { _mode = m; }
  void set_effort_level(PLACER::EffortLevel l) { _effort_level = l; }
  // 为布局做好准备
  bool ready_for_place(string output_file);

protected:
  // 初始化一块芯片
  void build_fpga();
  // 初始化布局
  bool init_place();
  // 初始化布局carrychain,lut6,特殊site
  void init_place_carrychain(CarryChainInference::CarryChains &carrys);
  void init_place_lut6(LUT6Inference::LUT6s &lut6s);
  bool init_place_site();

  bool find_init_pos_for_carrychain(int chain_length, std::vector<Point> &pos);

  void check_and_save_cst();
  // 检查并保存布局信息
  void check_and_save_place() {
    _nl_info.check_place();
    _nl_info.save_place();
  }

  typedef std::pair<bool, FLOORPLAN::SwapableType> Result;
  // from_site must not be LUT6, but may has carry chain part
  // from_obj has only 1 inst
  // pos_to.z is unknown after this function
  Result satisfy_cst_rules_for_site(SwapObject *from_obj, Point &pos_to,
                                    double rlim);
  // from_site must be LUT6
  // from_obj has 2 insts
  // pos_to.z is meaningless because both slices will be swapped
  Result satisfy_cst_rules_for_lut6(SwapObject *from_obj, Point &pos_to,
                                    double rlim);
  // from_site must be carry chain
  // from_site has several insts(a whole carry chain)
  // pos_to.z is known after this function
  Result satisfy_cst_rules_for_carrychain(SwapObject *from_obj, Point &pos_to,
                                          double rlim);

  typedef std::vector<PLCInstance *> SwapInsts;
  void update_qualified_pos_for_carrychain(int tile_col, int index);
  void update_qualified_pos_for_carrychain(SwapInsts &from_insts,
                                           SwapInsts &to_insts);
  //		void   update_qualified_pos_for_lut6();

  void swap_insts(SwapInsts &from_insts, SwapInsts &to_insts, Point &pos_from,
                  Point &pos_to, double rlim);
  void swap_insts_for_site(SwapObject *from_obj, Point &pos_to,
                           SwapInsts &from_insts, SwapInsts &to_insts,
                           FLOORPLAN::SwapableType to_type);
  void swap_insts_for_lut6(SwapObject *from_obj, Point &pos_to,
                           SwapInsts &from_insts, SwapInsts &to_insts,
                           FLOORPLAN::SwapableType to_type);
  void swap_insts_for_carrychain(SwapObject *from_obj, Point &pos_to,
                                 SwapInsts &from_insts, SwapInsts &to_insts,
                                 FLOORPLAN::SwapableType to_type);

  void maintain(bool keey_swap, SwapInsts &from_insts, SwapInsts &to_insts);

  void set_ignored_nets();
  void find_affected_nets(SwapInsts &insts, PLCNet::AffectedType t);
  void find_affected_nets(SwapInsts &from_insts, SwapInsts &to_insts);

  double compute_bb_cost();
  double compute_tcost(TEngine::WORK_MODE emode, double crit_exp);
  double compute_delta_bb_cost(SwapInsts &from_insts, SwapInsts &to_insts);
  double compute_delta_tcost(PLCInstance *target, PLCInstance *reference);
  double compute_delta_tcost(SwapInsts &from_insts, SwapInsts &to_insts);
  void update_tcost(PLCInstance *target, PLCInstance *reference);
  std::pair<double, double> recompute_cost(); // return <bb_cost, t_cost>
  /************************************************************************/
  /* 计算rlim */
  /************************************************************************/
  double max_rlim() const {
    return (double)std::max(_dev_info.device_scale().x,
                            _dev_info.device_scale().y);
  }
  /************************************************************************/
  /* 计算模拟退火过程的移动次数 */
  /************************************************************************/
  int num_swap_try() const {
    return (int)(_effort_level *
                 pow(_nl_info.design()->top_module()->num_instances(), 1.3333));
  }

  virtual FLOORPLAN::SwapableType find_to(SwapObject *from_obj, Point &pos_to,
                                          double rlim);

  // int index_of_carry_chain(int zpos);

private:
  PLACER::PlaceMode _mode;
  PLACER::EffortLevel _effort_level;

  NLInfo _nl_info;
  DeviceInfo _dev_info;

  Matrix<Tile> _fpga;
};

} // namespace Place
} // namespace FDU

#endif