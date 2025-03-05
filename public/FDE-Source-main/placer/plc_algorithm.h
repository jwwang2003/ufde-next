#ifndef PLCALGORITHM_H
#define PLCALGORITHM_H

#include "plc_args.h"
#include "plc_floorplan.h"
namespace FDU {
namespace Place {

using COS::TDesign;

/************************************************************************/
/* 模拟退火算法实现，cost的计算在FloorPlan，两者结合完成布局 */
/************************************************************************/
class SAPlacer {
public:
  explicit SAPlacer(TDesign *design)
      : _design(design), _mode(PLACER::BOUNDING_BOX),
        _floorplan(new Floorplan(_design)) {}

  ~SAPlacer() { delete _floorplan; }
  // 设置布局算法模式：BB，Timing-Driven
  void set_place_mode(PLACER::PlaceMode m) {
    _mode = m;
    _floorplan->set_place_mode(_mode);
  }
  void set_effort_level(PLACER::EffortLevel l) {
    _floorplan->set_effort_level(l);
  }
  // 完成模拟退火过程
  void try_place(PlaceArgs args);

protected:
  // 判断是否可以交换
  bool assert_swap(double delta_cost, double t);
  // 交换函数，同时计算cost
  bool try_swap(double t, double rlim, double &cost, double &bb_cost,
                double &tcost,
                // for timing-driven
                double inv_pre_bb_cost, double inv_pre_tcost);

  double get_std_dev(int n, double sum_of_square, double av);
  // 计算初始温度
  double starting_t(double rlim, double &cost, double &bb_cost, double &tcost,
                    // for timing-driven
                    double inv_pre_bbcost, double inv_pre_tcost,
                    bool isAllFixed);
  // 更新温度
  double update_t(double t, double rlim, double success_rat);
  // 更新r_lim
  double update_rlim(double rlim, double success_rat);
  // 退出模拟退火过程
  bool exit_sa_alg(double t, double cost);

private:
  PLACER::PlaceMode _mode; // 布局模拟退火算法的选择BB，Timing-Driven

  TDesign *_design;      // 设计
  Floorplan *_floorplan; // 提供计算cost和交换的对象
};

} // namespace Place
} // namespace FDU

#endif