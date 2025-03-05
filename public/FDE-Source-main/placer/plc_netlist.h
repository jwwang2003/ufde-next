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
/* 布局中可交换对象的类                                                 */
/************************************************************************/
class SwapObject {
public:
  explicit SwapObject(FLOORPLAN::SwapableType t) : _type(t) {}

  typedef std::vector<PLCInstance *> PLCInsts;
  // 返回类型
  FLOORPLAN::SwapableType type() const { return _type; }
  // 往insts里面提价一个instance
  void fill(PLCInstance *inst) { _insts.push_back(inst); }
  // 设置insts vector
  void fill(const PLCInsts &insts) { _insts = insts; }
  // 得到insts vector
  const PLCInsts &plc_insts() const { return _insts; }
  // 得到该swap obj的逻辑位置
  Point base_logic_pos() const;
  // 得到该swap obj里面的site
  Site &base_loc_site() const;

  void init_qualified_pos_tile_col(int tile_col);
  void clr_qualified_pos(int tile_col, int z_pos);
  void add_qualified_pos(const int index, const Point &logic_pos);
  bool exist_qualified_pos(int rows, const Point &logic_pos, const int index);
  // 更新isnt vector的inst_idx的内容
  void update_place_info(int inst_idx, const Point &pos, Site &loc_site);

  typedef std::map<int, std::vector<std::vector<int>>> QualifiedPos;
  QualifiedPos &get_qualified_pos() { return _qualified_pos; }

private:
  FLOORPLAN::SwapableType _type;
  // if swap object is carry chain, _insts[0] = bottom inst(max x)
  // if swap object is lut6,		  _insts[0] = slice_0
  // 存放可交换obj的instance的集合
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
/* 布局所用设计网表的信息类                                             */
/************************************************************************/
class NLInfo {
public:
  // 布局中的宏单元结构
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
  // 得到设计网表
  TDesign *design() const { return _design; }
  // 检查保存布局结果
  void check_place();
  void save_place();
  // 增加一个carry chain类型的Swapobj
  void add_carry_swap_obj(SwapObject *obj) {
    _macro_info._carrychains[obj->plc_insts().size()].push_back(obj);
  }
  // 得到网表中的carry chains
  MacroInfo::CarryChains &carry_chains() { return _macro_info._carrychains; }
  // 得到可交换对象的数目
  //		int			  num_swap_objects()       const {
  // return _swap_objs.size(); }
  int num_swap_objects() const { return _num_swap_objs; }
  // 得到/剔除index的可交换对象
  SwapObject *swap_object(int index) { return _swap_objs[index]; }
  void pop_swap_object(int index) {
    _swap_objs[index] = _swap_objs.back();
    _swap_objs.pop_back();
    --_num_swap_objs;
  }
  // 增加一个可交换对象
  SwapObject *add_swap_object(FLOORPLAN::SwapableType t) {
    SwapObject *obj = new SwapObject(t);
    _swap_objs.push_back(obj);
    ++_num_swap_objs;
    return obj;
  }
  // 存储受影响的nets
  typedef std::vector<std::pair<PLCInstance *, PLCNet *>> AffectedNets;
  AffectedNets &affected_nets() { return _affected_nets; }
  void clr_affected_nets() { _affected_nets.clear(); }
  void add_affected_net(PLCInstance *inst, PLCNet *net) {
    _affected_nets.push_back(std::make_pair(inst, net));
  }
  // 统计用到的资源
  void classify_used_resource();
  // 输出统计网表资源信息
  void echo_netlist_resource();
  // 输出资源利用率
  void echo_resource_usage(DeviceInfo::RscStat &dev_rsc);
  // void          echo_summary_netlist_resource(DeviceInfo::RscStat& dev_rsc);
  void writeReport(DeviceInfo::RscStat &dev_rsc, string output_file);

private:
  TDesign *_design;      // 设计网表
  MacroInfo _macro_info; // 记录网表中的macro信息，目前是carry chain

  typedef std::map<std::string, int> RscStat; // 记录所用资源
  RscStat _rsc_stat;

  typedef std::vector<SwapObject *> SwapObjects; // 记录网表中的swap objs
  SwapObjects _swap_objs;
  int _num_swap_objs;

  AffectedNets _affected_nets; // 记录受影响的线网
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
/*	功能：清除该swap obj的insts的qualified pos
 *	参数：
 *		tile_col:int,列坐标
 *		z_pos:int,z坐标
 *	返回值：void
 *	说明：
 */
/************************************************************************/
inline void SwapObject::clr_qualified_pos(int tile_col, int index) {
  ////_qualified_pos是一个map<col,vector<set<Point>>>
  ////首先判断该swap obj里面在该列上，
  ////这个判断主要是为了在初始化的时候由于没有任何数据，所有需要初始化
  // if(!_qualified_pos.count(tile_col))
  //	_qualified_pos.insert(
  //		make_pair(tile_col, std::vector<std::set<Point>
  //>(DEVICE::NUM_SLICE_PER_TILE))
  //	);
  ////清除指定位置的_qualified_pos
  // else
  //	_qualified_pos[tile_col][z_pos].clear();
  // if(_qualified_pos.count(tile_col)&&_qualified_pos[tile_col][index].size()
  // !=0)
  _qualified_pos[tile_col][index].clear();
}
/************************************************************************/
/*	功能：增加一个该swap obj的insts的qualified pos
 *	参数：
 *		tile_col:int,列坐标
 *		z_pos:int,z坐标
 *	返回值：void
 *	说明：
 */
/************************************************************************/
inline void SwapObject::add_qualified_pos(const int index,
                                          const Point &logic_pos) {
  ////如果没有相应的列坐标数据，那么初始化一个
  // if(!_qualified_pos.count(tile_col))
  //	_qualified_pos.insert(
  //		make_pair(tile_col, std::vector<std::set<Point>
  //>(DEVICE::NUM_SLICE_PER_TILE))
  //	);
  ////增加一个
  //_qualified_pos[tile_col][z_pos].insert(logic_pos);

  //		_qualified_pos[tile_col][z_pos].insert(logic_pos);
  // 		if(!_qualified_pos.count(logic_pos.y))
  // 			_qualified_pos[logic_pos.y];
  _qualified_pos[logic_pos.y][index].push_back(logic_pos.x);
}
/************************************************************************/
/*	功能：判断该swap obj的insts的qualified pos是否存在一个给定的逻辑地址
 *	参数：
 *		tile_col:int,列坐标
 *		z_pos:int,z坐标
 *	返回值：void
 *	说明：
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