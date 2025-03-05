#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "log.h"
#include "utils.h"
#include "xmlutils.h"

#include <map>
#include <string>
#include <vector>

namespace FDU {
namespace NLF {

using namespace FDU::XML;

class Config {
public:
  typedef std::vector<string> vec_str;
  typedef std::vector<size_t> vec_int;
  typedef std::map<string, string> bit_map;
  typedef bit_map::value_type bit_pair;

  Config(const string &name = "") : _name(name) {}

  string name() const { return _name; }
  void set_name(const string &name) { _name = name; }

  vec_str lib_names() const { return _lib_name; }
  size_t num_lib_name() const { return _lib_name.size(); }
  void add_lib_name(const string &libName) { _lib_name.push_back(libName); }

  vec_int bit_poses() const { return _bit_pos; }
  size_t num_bit_pos() const { return _bit_pos.size(); }
  void add_bit_pos(size_t bitPos) { _bit_pos.push_back(bitPos); }

  bit_map bitmap() const { return _bit_map; }
  bit_pair find_bit_pair(const string &bit) const;
  void add_bit_pair(const string &bit, const string &op) {
    _bit_map.insert(make_pair(bit, op));
  }

#ifdef _DEBUG
  void debug_info() const;
#endif

private:
  string _name;      ///< name in xdl file
  vec_str _lib_name; ///< name in library.xml, may be multiple
  vec_int _bit_pos;  ///< effective bit position in xdl file
  bit_map _bit_map;  ///< config-bit to operation mapping
};

class ConfigRepo {
public:
  typedef std::map<string, Config *> conf_map;
  typedef std::map<string, conf_map> repo;

  void add_config(const string &block, const string &pm, Config *pmConfig) {
    _config[block][pm] = pmConfig;
  }
  Config::vec_str find_lib_names(const string &block, const string &pm) const;
  Config::vec_int find_bit_poses(const string &block, const string &pm) const;
  Config::bit_pair get_operation(const string &block,
                                 const string &config) const;

#ifdef _DEBUG
  void debug_info() const;
#endif

private:
  repo _config;
};

class ConfigLoader {
public:
  ConfigLoader(ConfigRepo *crepo) : _repo(crepo) {}
  // void load_config(const string& file_name);
  void load(const string &file_name);

private:
  virtual void load_block(xml_node *node);
  virtual void load_prim(xml_node *node);
  virtual void load_bit(xml_node *node, Config *pm_conf);

  ConfigRepo *_repo; // shared, do not need to deallocate
  string _cur_block;
};

inline Config::bit_pair Config::find_bit_pair(const string &bit) const {
  bit_map::const_iterator iter = _bit_map.find(bit);
  if (iter != _bit_map.end())
    return *iter;
  ASSERT(0, "no bit \"" + bit + "\" for \"" + _name + "\"");
}

} // namespace NLF
} // namespace FDU
#endif