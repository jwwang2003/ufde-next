#ifndef PLCALGORITHM_H
#define PLCALGORITHM_H

#include "plc_args.h"
#include "plc_floorplan.h"
namespace FDU {
namespace Place {

using COS::TDesign;

/************************************************************************/
/* ģ���˻��㷨ʵ�֣�cost�ļ�����FloorPlan�����߽����ɲ��� */
/************************************************************************/
class SAPlacer {
public:
  explicit SAPlacer(TDesign *design)
      : _design(design), _mode(PLACER::BOUNDING_BOX),
        _floorplan(new Floorplan(_design)) {}

  ~SAPlacer() { delete _floorplan; }
  // ���ò����㷨ģʽ��BB��Timing-Driven
  void set_place_mode(PLACER::PlaceMode m) {
    _mode = m;
    _floorplan->set_place_mode(_mode);
  }
  void set_effort_level(PLACER::EffortLevel l) {
    _floorplan->set_effort_level(l);
  }
  // ���ģ���˻����
  void try_place(PlaceArgs args);

protected:
  // �ж��Ƿ���Խ���
  bool assert_swap(double delta_cost, double t);
  // ����������ͬʱ����cost
  bool try_swap(double t, double rlim, double &cost, double &bb_cost,
                double &tcost,
                // for timing-driven
                double inv_pre_bb_cost, double inv_pre_tcost);

  double get_std_dev(int n, double sum_of_square, double av);
  // �����ʼ�¶�
  double starting_t(double rlim, double &cost, double &bb_cost, double &tcost,
                    // for timing-driven
                    double inv_pre_bbcost, double inv_pre_tcost,
                    bool isAllFixed);
  // �����¶�
  double update_t(double t, double rlim, double success_rat);
  // ����r_lim
  double update_rlim(double rlim, double success_rat);
  // �˳�ģ���˻����
  bool exit_sa_alg(double t, double cost);

private:
  PLACER::PlaceMode _mode; // ����ģ���˻��㷨��ѡ��BB��Timing-Driven

  TDesign *_design;      // ���
  Floorplan *_floorplan; // �ṩ����cost�ͽ����Ķ���
};

} // namespace Place
} // namespace FDU

#endif