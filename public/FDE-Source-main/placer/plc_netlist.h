#ifndef PLCNETLIST_H
#define PLCNETLIST_H

#include "plc_device.h"

#include <map>
#include <set>
#include <vector>

namespace COS {
class TDesign;
}

namespace FDU {
namespace Place {

using namespace COS;

class PLCNet;
class PLCInstance;
struct Site;

/************************************************************************/
/* �����пɽ����������                                                 */
/************************************************************************/
class SwapObject {
public:
  explicit SwapObject(FLOORPLAN::SwapableType t) : _type(t) {}

  typedef std::vector<PLCInstance *> PLCInsts;
  // ��������
  FLOORPLAN::SwapableType type() const { return _type; }
  // ��insts�������һ��instance
  void fill(PLCInstance *inst) { _insts.push_back(inst); }
  // ����insts vector
  void fill(const PLCInsts &insts) { _insts = insts; }
  // �õ�insts vector
  const PLCInsts &plc_insts() const { return _insts; }
  // �õ���swap obj���߼�λ��
  Point base_logic_pos() const;
  // �õ���swap obj�����site
  Site &base_loc_site() const;

  void init_qualified_pos_tile_col(int tile_col);
  void clr_qualified_pos(int tile_col, int z_pos);
  void add_qualified_pos(const int index, const Point &logic_pos);
  bool exist_qualified_pos(int rows, const Point &logic_pos, const int index);
  // ����isnt vector��inst_idx������
  void update_place_info(int inst_idx, const Point &pos, Site &loc_site);

  typedef std::map<int, std::vector<std::vector<int>>> QualifiedPos;
  QualifiedPos &get_qualified_pos() { return _qualified_pos; }

private:
  FLOORPLAN::SwapableType _type;
  // if swap object is carry chain, _insts[0] = bottom inst(max x)
  // if swap object is lut6,		  _insts[0] = slice_0
  // ��ſɽ���obj��instance�ļ���
  PLCInsts _insts;

  // map::key = col, vec::idx = z pos, Point = logic pos
  // qualified pos means :
  //	 1. lowest pos for carry chain
  //	 2. leftest(s0) pos for lut6
  //	 3. ... if more macros exist in fpga
  //		typedef std::map<int, std::vector<std::set<Point> > >
  // QualifiedPos;

  QualifiedPos _qualified_pos;

  // vector<vector<Point>>					_pos_vector;
};

/************************************************************************/
/* ������������������Ϣ��                                             */
/************************************************************************/
class NLInfo {
public:
  // �����еĺ굥Ԫ�ṹ
  struct MacroInfo {
    // map<length, chains whose length equals the key>
    typedef std::map<int, std::vector<SwapObject *>> CarryChains;
    CarryChains _carrychains;
  };

  explicit NLInfo(TDesign *d) : _design(d), _num_swap_objs(0) {}
  virtual ~NLInfo() {
    for (SwapObject *o : _swap_objs)
      delete o;
  }
  // �õ��������
  TDesign *design() const { return _design; }
  // ��鱣�沼�ֽ��
  void check_place();
  void save_place();
  // ����һ��carry chain���͵�Swapobj
  void add_carry_swap_obj(SwapObject *obj) {
    _macro_info._carrychains[obj->plc_insts().size()].push_back(obj);
  }
  // �õ������е�carry chains
  MacroInfo::CarryChains &carry_chains() { return _macro_info._carrychains; }
  // �õ��ɽ����������Ŀ
  //		int			  num_swap_objects()       const {
  // return _swap_objs.size(); }
  int num_swap_objects() const { return _num_swap_objs; }
  // �õ�/�޳�index�Ŀɽ�������
  SwapObject *swap_object(int index) { return _swap_objs[index]; }
  void pop_swap_object(int index) {
    _swap_objs[index] = _swap_objs.back();
    _swap_objs.pop_back();
    --_num_swap_objs;
  }
  // ����һ���ɽ�������
  SwapObject *add_swap_object(FLOORPLAN::SwapableType t) {
    SwapObject *obj = new SwapObject(t);
    _swap_objs.push_back(obj);
    ++_num_swap_objs;
    return obj;
  }
  // �洢��Ӱ���nets
  typedef std::vector<std::pair<PLCInstance *, PLCNet *>> AffectedNets;
  AffectedNets &affected_nets() { return _affected_nets; }
  void clr_affected_nets() { _affected_nets.clear(); }
  void add_affected_net(PLCInstance *inst, PLCNet *net) {
    _affected_nets.push_back(std::make_pair(inst, net));
  }
  // ͳ���õ�����Դ
  void classify_used_resource();
  // ���ͳ��������Դ��Ϣ
  void echo_netlist_resource();
  // �����Դ������
  void echo_resource_usage(DeviceInfo::RscStat &dev_rsc);
  // void          echo_summary_netlist_resource(DeviceInfo::RscStat& dev_rsc);
  void writeReport(DeviceInfo::RscStat &dev_rsc, string output_file);

private:
  TDesign *_design;      // �������
  MacroInfo _macro_info; // ��¼�����е�macro��Ϣ��Ŀǰ��carry chain

  typedef std::map<std::string, int> RscStat; // ��¼������Դ
  RscStat _rsc_stat;

  typedef std::vector<SwapObject *> SwapObjects; // ��¼�����е�swap objs
  SwapObjects _swap_objs;
  int _num_swap_objs;

  AffectedNets _affected_nets; // ��¼��Ӱ�������
};

inline void SwapObject::init_qualified_pos_tile_col(int tile_col) {
  if (!_qualified_pos.count(tile_col))
    _qualified_pos.insert(
        //				make_pair(tile_col,
        // std::vector<std::set<Point> >(DEVICE::NUM_SLICE_PER_TILE))
        make_pair(tile_col,
                  std::vector<std::vector<int>>(DEVICE::NUM_CARRY_PER_TILE)));
}

//////////////////////////////////////////////////////////////////////////
// inline functions
/************************************************************************/
/*	���ܣ������swap obj��insts��qualified pos
 *	������
 *		tile_col:int,������
 *		z_pos:int,z����
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
inline void SwapObject::clr_qualified_pos(int tile_col, int index) {
  ////_qualified_pos��һ��map<col,vector<set<Point>>>
  ////�����жϸ�swap obj�����ڸ����ϣ�
  ////����ж���Ҫ��Ϊ���ڳ�ʼ����ʱ������û���κ����ݣ�������Ҫ��ʼ��
  // if(!_qualified_pos.count(tile_col))
  //	_qualified_pos.insert(
  //		make_pair(tile_col, std::vector<std::set<Point>
  //>(DEVICE::NUM_SLICE_PER_TILE))
  //	);
  ////���ָ��λ�õ�_qualified_pos
  // else
  //	_qualified_pos[tile_col][z_pos].clear();
  // if(_qualified_pos.count(tile_col)&&_qualified_pos[tile_col][index].size()
  // !=0)
  _qualified_pos[tile_col][index].clear();
}
/************************************************************************/
/*	���ܣ�����һ����swap obj��insts��qualified pos
 *	������
 *		tile_col:int,������
 *		z_pos:int,z����
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
inline void SwapObject::add_qualified_pos(const int index,
                                          const Point &logic_pos) {
  ////���û����Ӧ�����������ݣ���ô��ʼ��һ��
  // if(!_qualified_pos.count(tile_col))
  //	_qualified_pos.insert(
  //		make_pair(tile_col, std::vector<std::set<Point>
  //>(DEVICE::NUM_SLICE_PER_TILE))
  //	);
  ////����һ��
  //_qualified_pos[tile_col][z_pos].insert(logic_pos);

  //		_qualified_pos[tile_col][z_pos].insert(logic_pos);
  // 		if(!_qualified_pos.count(logic_pos.y))
  // 			_qualified_pos[logic_pos.y];
  _qualified_pos[logic_pos.y][index].push_back(logic_pos.x);
}
/************************************************************************/
/*	���ܣ��жϸ�swap obj��insts��qualified pos�Ƿ����һ���������߼���ַ
 *	������
 *		tile_col:int,������
 *		z_pos:int,z����
 *	����ֵ��void
 *	˵����
 */
/************************************************************************/
inline bool SwapObject::exist_qualified_pos(int rows, const Point &logic_pos,
                                            const int index) {
  // if(_qualified_pos.count(logic_pos.y))
  //	return _qualified_pos[logic_pos.y][logic_pos.z].count(logic_pos);
  // else
  //	return false;

  std::vector<bool> flags(rows, false);
  size_t carry_size = DEVICE::carry_chain[0].size();
  for (int row : _qualified_pos[logic_pos.y][index])
    flags[row] = true;
  return flags[logic_pos.x];
}

} // namespace Place
} // namespace FDU

#endif