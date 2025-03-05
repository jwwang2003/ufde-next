#ifndef PLCFACTORY_H
#define PLCFACTORY_H

#include "netlist.hpp"
#include "plc_boundingbox.h"
#include "plc_device.h"
#include "tnetlist.hpp"

namespace FDU {
namespace Place {

// using COS::NLList;
// using COS::NLContainer;
using namespace COS;
//  	using COS::Instance;
//  	using COS::Net;
//  	using COS::Module;
//  	using COS::Library;
// using COS::NLFactory;

/************************************************************************/
/* ��ʾ����������instance����                                           */
/************************************************************************/
class PLCInstance : public Instance {
public:
  PLCInstance(const std::string &name, Module &instof, Module &owner)
      : Instance(name, &instof, &owner), _curr_loc_site(nullptr),
        _curr_logic_pos(), _is_fixed(false), _swapable_type(FLOORPLAN::SITE) {}
  // �趨��instance�Ĵ���site�����ﱣ��������ָ�룬һ����ԭ���ģ�һ���ǵ�ǰ��
  void set_curr_loc_site(Site *site) {
    _past_loc_site = _curr_loc_site;
    _curr_loc_site = site;
  }
  // �õ���ǰ��site
  Site *curr_loc_site() const { return _curr_loc_site; }
  // �õ���ǰ��site
  Site *past_loc_site() const { return _past_loc_site; }
  // ��λsite
  void reset_loc_site() { _curr_loc_site = _past_loc_site; }

  // ���õ�ǰ�߼���ַ
  void set_curr_logic_pos(const Point &pos) {
    _past_logic_pos = _curr_logic_pos;
    _curr_logic_pos = pos;
  }
  // �õ���ǰ/��ǰ�߼���ַ
  Point curr_logic_pos() const { return _curr_logic_pos; }
  Point past_logic_pos() const { return _past_logic_pos; }
  // ��λ�߼���ַ
  void reset_logic_pos() { _curr_logic_pos = _past_logic_pos; }

  // fix����Լ���ļ��й̶���Instance
  void fix_inst() { _is_fixed = true; }
  void loose_inst() { _is_fixed = false; }
  bool is_fixed() const { return _is_fixed; }

  Site::SiteType site_type() const { return _curr_loc_site->_type; }
  // ����/�õ���isntance�Ŀɽ�������
  void set_swapable_type(FLOORPLAN::SwapableType t) { _swapable_type = t; }
  FLOORPLAN::SwapableType swapable_type() const { return _swapable_type; }
  // �ж��Ƿ���macro type
  bool is_macro() const {
    return _swapable_type == FLOORPLAN::LUT6 ||
           _swapable_type == FLOORPLAN::CARRY_CHAIN;
  }

private:
  // �洢��һ���͵�ǰ������site
  Site *_curr_loc_site, *_past_loc_site;
  // �洢��ǰ����һ�����߼���ַ
  Point _curr_logic_pos, _past_logic_pos;
  // �Ƿ���fix
  bool _is_fixed;
  // ��instance�Ŀɽ�������
  FLOORPLAN::SwapableType _swapable_type;
};

/************************************************************************/
/* ��ʾ����������net����                                                */
/************************************************************************/
class PLCNet : public Net {
public:
  enum AffectedType { NONE = 0, FROM = 1, TO = 2 };

  PLCNet(const std::string &name, COS::NetType type, Module &owner)
      : Net(name, type, &owner, nullptr), _is_ignored(false), _affected_type(NONE),
        _bounding_box(new BoundingBox(this)), _total_tcost(0.) {}
  ~PLCNet() { delete _bounding_box; }
  // ����/�õ�netΪignored
  void set_ignored() { _is_ignored = true; }
  bool is_ignored() const { return _is_ignored; }

  void save_bounding_box() { _bounding_box->save_bounding_box(); }
  void restore_bounding_box() { _bounding_box->restore_bounding_box(); }

  double compute_tcost(double dmax, double crit_exp);
  double get_total_tcost() const { return _total_tcost; }

  void set_pre_bb_cost(double cost) { _bounding_box->set_pre_bb_cost(cost); }
  void reset_pre_bb_cost() { _bounding_box->reset_pre_bb_cost(); }
  double pre_bb_cost() const { return _bounding_box->pre_bb_cost(); }
  double bb_cost() const { return _bounding_box->bb_cost(); }
  double compute_bb_cost(double inv_of_chan_width) {
    return _bounding_box->compute_bb_cost(inv_of_chan_width);
  }

  void compute_bounding_box(const Point &device_scale) {
    _bounding_box->compute_bounding_box(device_scale);
  }
  void update_bounding_box(const Point &pos_from, const Point &pos_to,
                           const Point &device_scale) {
    _bounding_box->update_bounding_box(device_scale, pos_from, pos_to);
  }
  // ������Ӱ������������
  void set_affected_type(AffectedType type) { _affected_type |= type; }
  void reset_affected_type() { _affected_type = 0; }
  int affected_type() const { return _affected_type; }

  typedef std::vector<PLCInstance *> ConnectedInsts;
  const ConnectedInsts &connected_insts() const { return _connected_insts; }
  size_t num_connected_insts() const { return _connected_insts.size(); }
  void store_connected_insts();

private:
  bool _is_ignored;
  int _affected_type;
  BoundingBox *_bounding_box;

  double _total_tcost;

  ConnectedInsts _connected_insts;
};

class PLCModule : public Module {
public:
  PLCModule(const std::string &name, const std::string &type, Library *owner)
      : Module(name, type, owner) {}

  typedef PtrList<Instance>::typed<PLCInstance>::range instances_type;
  typedef PtrList<Instance>::typed<PLCInstance>::const_range
      const_instances_type;
  typedef PtrList<Instance>::typed<PLCInstance>::iterator instance_iter;
  typedef PtrList<Instance>::typed<PLCInstance>::const_iterator
      const_instance_iter;

  instances_type instances() {
    return static_cast<instances_type>(Module::instances());
  }
  const_instances_type instances() const {
    return static_cast<const_instances_type>(Module::instances());
  }

  typedef PtrVector<Net>::typed<PLCNet>::range nets_type;
  typedef PtrVector<Net>::typed<PLCNet>::const_range const_nets_type;
  typedef PtrVector<Net>::typed<PLCNet>::iterator net_iter;
  typedef PtrVector<Net>::typed<PLCNet>::const_iterator const_net_iter;

  nets_type nets() { return static_cast<nets_type>(Module::nets()); }
  const_nets_type nets() const {
    return static_cast<const_nets_type>(Module::nets());
  }
};

/************************************************************************/
/* ������������Ԫ�صĹ��� */
/************************************************************************/
class PLCFactory : public TimingFactory {
public:
  ~PLCFactory() {}

  virtual Module *make_module(const string &name, const string &type,
                              Library *owner) {
    return new PLCModule(name, type, owner);
  }

  virtual Instance *make_instance(const string &name, Module *down_module,
                                  Module *owner) {
    return new PLCInstance(name, *down_module, *owner);
  }

  virtual Net *make_net(const string &name, NetType type, Module *owner,
                        Bus *bus) {
    return new PLCNet(name, type, *owner);
  }
};

} // namespace Place
} // namespace FDU

#endif