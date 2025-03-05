#include "config.hpp"
#include "zfstream.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <rapidxml/rapidxml_utils.hpp>

namespace FDU {
namespace NLF {

using namespace std;
using namespace boost;
using FDU::ifstrm;

#ifdef _DEBUG
void Config::debug_info() const {
  cout << "Primitive: " << _name << endl;

  cout << "Lib Name: ";
  for (string libName : _lib_name) {
    cout << libName << " ";
  }
  cout << endl;

  cout << "Bit Position: ";
  for (size_t bitPos : _bit_pos) {
    cout << bitPos << " ";
  }
  cout << endl;

  for (map<string, string>::const_iterator iter = _bit_map.begin();
       iter != _bit_map.end(); iter++) {
    cout << "Config: " << iter->first << " " << iter->second << endl;
  }
}

void ConfigRepo::debug_info() const {
  for (const auto& block : _config) {
    cout << "Block: " << block.first << endl;
    for (const auto& pm : block.second) {
      cout << "Primitive: " << pm.first << endl;
      pm.second->debug_info();
    }
  }
}
#endif

// 	void ConfigLoader::load_config(const string& file_name)
// 	{
// 		zifstream zifs(file_name.c_str());
// 		if (zifs)
// 			load(zifs);
// 		else
// 		{
// 			std::ifstream ifs(file_name.c_str());
// 			load(ifs);
// 		}
// 	}
void ConfigLoader::load(const string &file_name) {
  typedef rapidxml::file<> xml_file;
  ifstrm istrm(file_name.c_str());
  xml_file file(istrm);
  xml_document doc;
  doc.parse<0>(file.data());
  xml_node *node = doc.first_node();
  if (node->name() == string("library"))
    foreach_child(block_node, node, "block") load_block(block_node);
  else
    ASSERT(
        0,
        "Parse failed. Config lib format error, first node should be library.");
}

void ConfigLoader::load_block(xml_node *node) {
  _cur_block = get_attribute(node, "name");
  ASSERT(_cur_block.size(), "parse config lib failed. block name not found");
  foreach_child(prim_node, node, "primitive") load_prim(prim_node);
}

void ConfigLoader::load_prim(xml_node *node) {
  string aname(get_attribute(node, "name"));
  ASSERT(aname.size(), "parse config lib failed. primitive name not found");
  Config *prim = new Config(aname);
  string alib_names(get_attribute(node, "libname"));
  ASSERT(alib_names.size(),
         "parse config lib failed. lib name not found at primitive " + aname);
  string aposes(get_attribute(node, "pos"));
  ASSERT(aposes.size(),
         "parse config lib failed. pos not found at primitive " + aname);

  char_separator<char> sep(" \t\n|");
  tokenizer<char_separator<char>> lib_tokens(alib_names, sep);
  for (const string &token : lib_tokens)
    prim->add_lib_name(token);
  tokenizer<char_separator<char>> pos_tokens(aposes, sep);
  for (const string &token : pos_tokens)
    prim->add_bit_pos(lexical_cast<size_t>(token));

  foreach_child(bit_node, node, "config") load_bit(bit_node, prim);

  _repo->add_config(_cur_block, aname, prim);
}

void ConfigLoader::load_bit(xml_node *node, Config *pm_conf) {
  string bit_key(get_attribute(node, "bit"));
  ASSERT(bit_key.size(), "parse config lib failed. bit name not found");
  string bit_op(get_attribute(node, "op"));
  ASSERT(bit_op.size(), "parse config lib failed. operation value not found");
  pm_conf->add_bit_pair(bit_key, bit_op);
}

Config::vec_str ConfigRepo::find_lib_names(const string &block,
                                           const string &prim) const {
  string block_name(to_lower_copy(block));
  string prim_name(to_upper_copy(prim));
  repo::const_iterator block_it = _config.find(block_name);
  if (block_it == _config.end()) {
    FDU_LOG(WARN) << "fail to lookup \"" << block_name
                  << "\" in configuration rule data base";
    return Config::vec_str();
  }

  conf_map::const_iterator prim_it = block_it->second.find(prim_name);
  if (prim_it == block_it->second.end()) {
    if (prim_name != "_BEL_PROP")
      FDU_LOG(WARN) << "fail to lookup \"" << block_name << "::" << prim_name
                    << "\" in configuration rule data base";
    return Config::vec_str();
  }

  return prim_it->second->lib_names();
}

Config::vec_int ConfigRepo::find_bit_poses(const string &block,
                                           const string &prim) const {
  string block_name(to_lower_copy(block));
  string prim_name(to_upper_copy(prim));
  repo::const_iterator block_it = _config.find(block_name);
  if (block_it == _config.end()) {
    FDU_LOG(WARN) << "fail to lookup \"" << block_name
                  << "\" in configuration rule data base" << endl;
    return Config::vec_int();
  }

  conf_map::const_iterator prim_it = block_it->second.find(prim_name);
  if (prim_it == block_it->second.end()) {
    if (prim_name != "_BEL_PROP")
      FDU_LOG(WARN) << "fail to lookup \"" << block_name << "::" << prim_name
                    << "\" in configuration rule data base" << endl;
    return Config::vec_int();
  }

  return prim_it->second->bit_poses();
}

Config::bit_pair ConfigRepo::get_operation(
    const string &block,     ///< instance module name (SLICE or IOB)
    const string &merge_conf ///< config.name() + "::" + config.value()
) const {
  // <1> separate merge config first
  // configuration vBit are separated using "::", ":", etc.
  // note that "#" are also removed, so that only "OFF" will appear
  char_separator<char> sep(":#\n\t");
  tokenizer<char_separator<char>> tokens(merge_conf, sep);
  typedef tokenizer<char_separator<char>>::iterator Iter;
  vector<string> conf_sep_list; // separated configuration vBit (vBit[0]=BXMUX,
                                // vBit[1]=OFF, etc.)
  for (Iter iter = tokens.begin(); iter != tokens.end(); iter++) {
    conf_sep_list.push_back(*iter);
  }

  string prim(to_upper_copy(conf_sep_list[0])); // primitive name
  string block_name(to_lower_copy(block));
  Config::vec_int bit_poses = find_bit_poses(block_name, prim);
  if (bit_poses.size() == 0)
    return Config::bit_pair("none", "none");

  size_t match = 0;                             // total number of matched cases
  string bit;                                   // actual valid bit value
  string operation;                             // actual operation
  for (size_t i = 0; i < bit_poses.size(); ++i) // for every pos value
  {
    size_t pos = bit_poses[i];
    if (pos >=
        conf_sep_list.size()) { // then this pos value is invalid, skip it
      continue;
    }
    bit = conf_sep_list[pos]; // actual bit

    // calling find_bit_poses() has already guaranteed _config[block][pm] exists
    Config::bit_map bit_map =
        _config.find(block_name)->second.find(prim)->second->bitmap();

    if (bit_map.size() == 0) { // case when no bit appears
      FDU_LOG(WARN) << "no config bit rule for " << block_name << "::" << prim;
      return Config::bit_pair("none", "none");
    }

    if (bit_map.size() == 1 &&
        bit_map.find("any") != bit_map.end()) { // where the bit is name, etc.
      return Config::bit_pair(bit, bit_map["any"]);
    }

    Config::bit_map::const_iterator bit_it = bit_map.find(bit);
    if (bit_it != bit_map.end()) { // find one match
      operation = bit_it->second;
      ++match;
    }
  }

  // should exactly match one bit
  if (match != 1) {
    for (const string &op : conf_sep_list) { // find one OFF
      if (op == "OFF") {
        match = 1;
        operation = "remove";
        break;
      }
    }
  }
  ASSERTS(match == 1);
  return Config::bit_pair(bit, operation);
}
} // namespace NLF
} // namespace FDU