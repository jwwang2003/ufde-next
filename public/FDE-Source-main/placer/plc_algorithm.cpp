#include "plc_algorithm.h"
#include "plc_args.h"
#include "plc_utils.h"
#include <boost/tuple/tuple.hpp>

namespace FDU {
namespace Place {

using namespace std;
using namespace COS;
/************************************************************************/
/*	���ܣ��ж��Ƿ���Խ�����������Խ�������ture�����򷵻�false
 *	������
                delta_cost: �����Ժ�cost�Ĳ�ֵ
                t:			�����¶�
 *	����ֵ��bool
 *	˵�����ɽ��ܵĽ���������
 *			1. delta_cost<0
 *			2. exp(-delta_cost/t) > rand
 */
/************************************************************************/
bool SAPlacer::assert_swap(double delta_cost, double t) {
  double fnum, prob_num;

  if (delta_cost <= 0)
    return true;

  if (t == 0.)
    return false;

  fnum = (double)rand() / RAND_MAX;
  prob_num = exp(-delta_cost / t);

  if (prob_num > fnum)
    return true;
  else
    return false;
}

/************************************************************************/
/*	���ܣ�	����һ�ν���
 *	������
                        t: ��ǰ�¶�
                        rlim������λ��Լ��
                        cost����ʱ��cost
                        bb_cost:bouding box��cost
                        tcost:ʱ��������cost
                        inv_pre_bb_cost:
                        inv_pre_tcost:
 *	����ֵ��bool
 *	˵����
 */
/************************************************************************/
bool SAPlacer::try_swap(double t, double rlim, double &cost, double &bb_cost,
                        double &tcost, double inv_pre_bb_cost,
                        double inv_pre_tcost) {
  vector<PLCInstance *> from_insts, to_insts;
  Point pos_from, pos_to;
  // ��������instance
  _floorplan->swap_insts(from_insts, to_insts, pos_from, pos_to, rlim);

  double delta_cost = 0., delta_bb_cost = 0., delta_tcost = 0.;
  // ���¼���cost
  delta_bb_cost = _floorplan->compute_delta_bb_cost(from_insts, to_insts);
  if (_mode == PLACER::TIMING_DRIVEN) {
    delta_tcost = _floorplan->compute_delta_tcost(from_insts, to_insts);

    delta_cost =
        (1 - PLACER::TIMING_TRADE_OFF) * delta_bb_cost * inv_pre_bb_cost +
        PLACER::TIMING_TRADE_OFF * delta_tcost * inv_pre_tcost;
  } else {
    delta_cost = delta_bb_cost;
  }
  // �ж��Ƿ���Խ���
  bool keep_swap = assert_swap(delta_cost, t);
  if (keep_swap) {
    cost += delta_cost;
    bb_cost += delta_bb_cost;
    if (_mode == PLACER::TIMING_DRIVEN)
      tcost += delta_tcost;
  }
  // ����ʵ���ԵĽ������ǻ���
  _floorplan->maintain(keep_swap, from_insts, to_insts);
  // �����Ƿ񽻻��ɹ�
  return keep_swap;
}
/************************************************************************/
/*	���ܣ�	�õ���׼����
*	������
                n:
                sum_of_square:
                av:
                inv_pre_tcost:
*	����ֵ��double:
*	˵����
*/
/************************************************************************/
double SAPlacer::get_std_dev(int n, double sum_of_square, double av) {
  double std_dev;

  if (n <= 1)
    std_dev = 0.;
  else
    std_dev = (sum_of_square - n * av * av) / (double)(n - 1);

  /* Very small variances sometimes round negative */
  if (std_dev > 0.)
    std_dev = sqrt(std_dev);
  else
    std_dev = 0.;

  return std_dev;
}

/************************************************************************/
/*	���ܣ�	�����ʼ�¶ȼ���cost
*	������
                t: ��ǰ�¶�
                rlim��
                cost��
                bb_cost:
                tcost:
                inv_pre_bb_cost:
                inv_pre_tcost:
*	����ֵ��bool
*	˵����
*/
/************************************************************************/
double SAPlacer::starting_t(double rlim, double &cost, double &bb_cost,
                            double &tcost, double inv_pre_bbcost,
                            double inv_pre_tcost, bool isAllFixed) {
  int num_swap_try = isAllFixed ? 0 : _design->top_module()->num_instances();
  int num_swap_accepted = 0;
  double std_dev = 0., av = 0., sum_of_square = 0.;

  for (int i = 0; i < num_swap_try; ++i) {
    if (try_swap(PLACER::HIGHEST_T, rlim, cost, bb_cost, tcost, inv_pre_bbcost,
                 inv_pre_tcost)) {
      ++num_swap_accepted;
      av += cost;
      sum_of_square += cost * cost;
    }
  }

  if (num_swap_accepted != 0)
    av /= (double)num_swap_accepted;
  else
    av = 0.;

  std_dev = get_std_dev(num_swap_accepted, sum_of_square, av);

  return (20. * std_dev);
}
/************************************************************************/
/*	���ܣ�	�����´��¶�
*	������
                t: �¶�
                rlim��Լ��limit
                success_rat�� �ɹ�����
*	����ֵ��double: ����Ҫ���µ��¶�
*	˵����
*/
/************************************************************************/
double SAPlacer::update_t(double t, double rlim, double success_rat) {
  if (success_rat > 0.96)
    return t * 0.5;
  else if (success_rat > 0.8)
    return t * 0.9;
  else if (success_rat > 0.15 || rlim > 1.)
    return t * 0.95;
  else
    return t * 0.8;
}
/************************************************************************/
/*	���ܣ�	�����´�rlim
*	������
                rlim��Լ��limit
                success_rat�� �ɹ�����
*	����ֵ��double: ���º��rlim
*	˵����
*/
/************************************************************************/
double SAPlacer::update_rlim(double rlim, double success_rat) {
  double upper_lim;
  double temp_rlim;

  temp_rlim = rlim * (1. - 0.44 + success_rat);
  upper_lim = _floorplan->max_rlim();

  temp_rlim = min(temp_rlim, upper_lim);
  temp_rlim = max(temp_rlim, (double)1.);

  return temp_rlim;
}

/************************************************************************/
/*	���ܣ�	�ж��Ƿ��˳�ģ���˻����
*	������
                t����ǰ�¶�
                cost�� ��ǰcost
*	����ֵ��bool
*	˵����
*/
/************************************************************************/
bool SAPlacer::exit_sa_alg(double t, double cost) {
  static double num_nets = (double)_design->top_module()->num_nets();

  if (t < 0.005 * cost / num_nets)
    return true;
  else
    return false;
}

/************************************************************************/
/*	���ܣ�	����ģ���˻��㷨
 *	������	void
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
void SAPlacer::try_place(PlaceArgs args) {
  bool isAllFixed;
  // ׼���ò������趫������������fpga��ͳ��������������Ϣ����������ṹ��
  isAllFixed = _floorplan->ready_for_place(args._output_nl);

  double cost = 0., bb_cost = 0., tcost = 0.;
  double inv_pre_bb_cost = 0., inv_pre_tcost = 0.;
  double crit_exp;
  // ����bb_cost
  bb_cost = _floorplan->compute_bb_cost();
  // ����ʱ������cost
  if (_mode == PLACER::TIMING_DRIVEN) {
    crit_exp = PLACER::DEFAULT_CRIT_EXP;
    tcost = _floorplan->compute_tcost(TEngine::REBUILD, crit_exp);

    cost = 1.;
    inv_pre_bb_cost = 1. / bb_cost;
    inv_pre_tcost = 1. / tcost;
  } else // PLACER::BOUNDING_BOX
    cost = bb_cost;

  int move_lim = _floorplan->num_swap_try();
  double rlim = _floorplan->max_rlim();

  double final_rlim = 1.;
  double inv_delta_rlim = 1. / (rlim - final_rlim);
  // �õ���ʼ���¶ȼ���cost
  double t = starting_t(rlim, cost, bb_cost, tcost, inv_pre_bb_cost,
                        inv_pre_tcost, isAllFixed);

#ifdef VERBOSE
  INFO(CONSOLE::INIT_COST % t % cost);
#else
  INFO(CONSOLE::INIT_COST % cost);
#endif
  int num_total_swap = 0, num_swap_to_recompute = 0, outer_iter_num = 0;
  int num_success;
  double av_cost, av_bb_cost, av_tcost, sum_of_square;
  double success_rat;
  // ģ���˻����
  INFO(CONSOLE::PROGRESS % "70" % "begin to perform place iteration");
  if (!isAllFixed) {
    while (!exit_sa_alg(t, cost)) {
      num_success = 0;
      av_cost = av_bb_cost = av_tcost = sum_of_square = 0.;

      if (_mode == PLACER::TIMING_DRIVEN) {
        tcost = _floorplan->compute_tcost(TEngine::INCREMENT, crit_exp);

        cost = 1.;
        inv_pre_bb_cost = 1. / bb_cost;
        inv_pre_tcost = 1. / tcost;
      }
      // ����move_lim��
      for (int i = 0; i < move_lim; ++i) {
        // �ɹ�����һ��
        if (try_swap(t, rlim, cost, bb_cost, tcost, inv_pre_bb_cost,
                     inv_pre_tcost)) {
          ++num_success;

          av_cost += cost;
          av_bb_cost += bb_cost;
          av_tcost += tcost;
          sum_of_square += cost * cost;
        }
      }

      ASSERT(cost >= 0., (CONSOLE::PLC_ERROR % "cost is negative."));

      num_total_swap += move_lim;
      num_swap_to_recompute += move_lim;
      // �������̫��λ���û���˳��˻���̣���ô���¼���cost
      if (num_swap_to_recompute > PLACER::MAX_SWAPS_BEFORE_RECOMPUTE) {
        num_swap_to_recompute = 0;

        boost::tie(bb_cost, tcost) = _floorplan->recompute_cost();
        if (_mode == PLACER::BOUNDING_BOX)
          cost = bb_cost;
      }

#ifdef UNUSED
      double std_dev;
      // unused
      if (num_success == 0) {
        av_cost = cost;
        av_bb_cost = bb_cost;
        av_tcost = tcost;
      } else {
        av_cost = cost / num_success;
        av_bb_cost = bb_cost / num_success;
        av_tcost = tcost / num_success;
      }
      // unused
      std_dev = get_std_dev(num_success, sum_of_square, av_cost);
#endif
      // �����¶ȣ�rlim
      success_rat = (double)num_success / move_lim;
      t = update_t(t, rlim, success_rat);
      rlim = update_rlim(rlim, success_rat);

      if (_mode == PLACER::TIMING_DRIVEN)
        crit_exp = (1.0 - (rlim - final_rlim) * inv_delta_rlim) * 7.0 + 1.0;

      // ��¼��������
      outer_iter_num++;
#ifdef VERBOSE
      INFO(CONSOLE::ITER_COST % outer_iter_num % t % rlim % cost);
#else
      if ((outer_iter_num) % 20 == 0)
        INFO(CONSOLE::ITER_COST % outer_iter_num % cost);
#endif
    } // end of "while(!exit_sa_alg(t, cost))"
  }

  // freeze out
  if (_mode == PLACER::TIMING_DRIVEN) {
    tcost = _floorplan->compute_tcost(TEngine::INCREMENT, crit_exp);

    inv_pre_bb_cost = 1. / bb_cost;
    inv_pre_tcost = 1. / tcost;
  }
  if (!isAllFixed) {
    INFO(CONSOLE::PROGRESS % "80" % "begin to perform final place iteration");
    for (int i = 0; i < move_lim; ++i)
      try_swap(t, rlim, cost, bb_cost, tcost, inv_pre_bb_cost, inv_pre_tcost);
  }
#ifdef VERBOSE
  INFO(CONSOLE::FINAL_COST % t % rlim % cost);
#else
  INFO(CONSOLE::FINAL_COST % cost);
#endif
  // ��鲢���沼����Ϣ
  _floorplan->check_and_save_place();
}

} // namespace Place
} // namespace FDU