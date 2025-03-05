#ifndef PLCDEVICE_H
#define PLCDEVICE_H

#include "matrix.h"
#include "plc_utils.h"

#include <map>

namespace FDU {
namespace Place {

class PLCInstance;
class Tile;
/************************************************************************/
/* Site�Ķ�����                                                         */
/************************************************************************/
struct Site {
  enum SiteType {
    IGNORE = -1,
    SLICE,
    TBUF,     // CENTER
    IOB,      // TOP, BOTTOM, LEFT, RIGHT
    BLOCKRAM, // BRAM
    GCLK,
    GCLKIOB,
    DLL, // CLKT, CLKB
    VCC,
    BUFGMUX,
    BRAM16,
    MULT18X18,
    DCM, // New site in V2
    NUM_OF_SITE_TYPE
  };

  SiteType _type;      // ��site������
  Tile *_owner;        // ��site�����Ǹ�Tile
  int _occ, _capacity; // ��site��ռ�õ���ľ������
  Point _logic_pos;    // ��site���߼�λ��

  typedef std::vector<PLCInstance *> OccInsts; // ��site��ռ�õ�instance
  OccInsts _occ_insts;

  typedef std::vector<Point> AvailablePos;
  AvailablePos _available_pos; // ��site��оƬ�ϵĿɽ���λ��

  Site(SiteType t, Tile *o)
      : _type(t), _owner(o), _occ(0), _capacity(0), _logic_pos() {}

  bool has_macro() const;                     // ��site�����Ƿ���macro��Ԫ
  bool has_fixed_insts() const;               // ��site�Ƿ���fix��Ԫ
  FLOORPLAN::SwapableType macro_type() const; // �õ���site��macro type

  // "get" here means some algorithm used in these functions, not simply get the
  // value so these function should be take serious
  int get_unocc_z_pos() const;
  PLCInstance *get_occ_inst(int idx) const;

  void pop_unavailable_pos(int pos_idx) {
    _available_pos[pos_idx] = _available_pos.back();
    _available_pos.pop_back();
  }
};
/************************************************************************/
/* Tile�Ķ�����                                                         */
/************************************************************************/
class Tile {
public:
  ~Tile() {
    for (Sites::value_type &s : _sites)
      delete s.second;
  }
  // ����/�õ�Tile������
  void set_type(const std::string &type) { _type = type; }
  std::string type() const { return _type; }
  // ����/�õ�Tile������λ��
  void set_phy_pos(const Point &phy_pos) { _phy_pos = phy_pos; }
  Point phy_pos() const { return _phy_pos; }
  // �õ������sites
  typedef std::map<Site::SiteType, Site *> Sites;
  Sites &sites() { return _sites; }
  // �ж��Ƿ����type�͵�site
  bool exist_site(Site::SiteType type) const { return _sites.count(type); }
  // �õ�ĳһ���͵�site
  Site &site(Site::SiteType type) { return *_sites[type]; }
  // ����ĳһ���͵�site
  Site &add_site(Site::SiteType type) {
    Site *site = new Site(type, this);
    _sites.insert(std::make_pair(type, site));
    return *site;
  }

private:
  std::string _type; // ��¼��Tile������
  Point _phy_pos;    // Phy position

  Sites _sites; // �洢�����sites,map
};

class DeviceInfo {
public:
  typedef std::map<Site::SiteType, int> RscStat;
  typedef std::map<Site::SiteType, std::vector<Point>> RscAvaPos;

  DeviceInfo() : _scale(), _inv_of_chan_width(DEVICE::INV_OF_CHAN_WIDTH) {}

  bool is_logic_site(Site::SiteType type) { return type > Site::IGNORE; }

  void set_device_scale(const Point &scale) { _scale = scale; }
  Point device_scale() const { return _scale; }

  void set_slice_num(const int num) { DEVICE::NUM_SLICE_PER_TILE = num; }
  void set_carry_num(const int num) { DEVICE::NUM_CARRY_PER_TILE = num; }
  void set_carry_chain(const vector<vector<int>> carry) {
    DEVICE::carry_chain = carry;
  }
  void set_lut_inputs(const int num) { DEVICE::NUM_LUT_INPUTS = num; }

  void set_inv_of_chan_width(double value) { _inv_of_chan_width = value; }
  double inv_of_chan_width() const { return _inv_of_chan_width; }

  double get_p2p_delay(PLCInstance *src, PLCInstance *sink);

  void sum_up_rsc_in_device();
  RscStat &rsc_in_device() { return _rsc_stat; }

  void add_rsc_ava_pos(Site::SiteType type, const Point &pos) {
    _rsc_available_pos[type].push_back(pos);
  }

  std::vector<Point> &rsc_ava_pos(Site::SiteType type) {
    return _rsc_available_pos[type];
  }

  void pop_rsc_ava_pos(Site::SiteType type, int index) {
    _rsc_available_pos[type][index] = _rsc_available_pos[type].back();
    _rsc_available_pos[type].pop_back();
  }

  typedef Matrix<double> DelayTable;
  typedef std::vector<DelayTable> DelayTables;
  static void load_delay_tables(const string &file, bool encrypt = false);
  static void resize_delay_table(int size) { _delay_tables.resize(size); }
  static DelayTable &delay_table(DEVICE::DelayTableType type) {
    return _delay_tables[type];
  }

  static Point lookup_phy_pos(const Point &p) { return _phy_pos_lut.at(p); }

protected:
  double lookup_delay_from_table(const DelayTable &table, int delta_row,
                                 int delta_col);

private:
  Point _scale;
  double _inv_of_chan_width;

  RscStat _rsc_stat;
  RscAvaPos _rsc_available_pos;

  static DelayTables _delay_tables;
  static Matrix<Point> _phy_pos_lut;
};

//////////////////////////////////////////////////////////////////////////
// streams

std::istream &operator>>(std::istream &s, Site::SiteType &type);
std::ostream &operator<<(std::ostream &s, Site::SiteType type);

} // namespace Place
} // namespace FDU

#endif