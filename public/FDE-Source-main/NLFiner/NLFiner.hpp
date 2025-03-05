#ifndef NETLIST_FINER_HPP
#define NETLIST_FINER_HPP

#include "config.hpp"
#include "rtnetlist.hpp"

namespace FDU {
namespace NLF {

using namespace COS;

class NLFiner {
  struct match_cell {
    match_cell(const string &name) : to_match(name) {}
    bool operator()(const Instance *t) {
      return to_match == t->down_module()->name();
    }

  private:
    string to_match;
  };

  typedef std::vector<string> vec_str;

public:
  NLFiner(Design *design, Design *cell_lib, ConfigRepo *repo)
      : _design(design), _cell_lib(cell_lib), _repo(repo) {}

  void refine();

private:
  void rebuild_verbose_netlist();
  void rebuild_verbose_instance(Instance *inst, std::map<string, size_t> &name_repo);
  void copy_template_cell_lib();
  void remove_unused_module();

  void refine_instance(Instance *inst);
  void handle_WSGEN(Module &newCell);
  void handle_FG(Instance *inst, const string &new_cell_name);

  void restructure(Module &cell, const vec_str &lib_names, string bit,
                   string op);

  void op_remove(Module &cell, const vec_str &lib_names);
  void op_mux(Module &cell, const vec_str &lib_names, const string &bit);
  void op_vccgnd(Module &cell, const vec_str &lib_names, const string &bit);
  void op_attr(Module &cell, const vec_str &lib_names, const string &bit,
               const string &property_name);
  void op_sweep(Module &cell);

  Design *_design;   // shared pointer of netlist
  Design *_cell_lib; // shared pointer of cell lib
  ConfigRepo *_repo; // shared pointer of config lib
};
} // namespace NLF
} // namespace FDU

#endif