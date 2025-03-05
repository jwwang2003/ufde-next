#ifndef _PACKER_H
#define _PACKER_H

#include "NetlistCSP.h"
#include "Transformer.h"

namespace PACK {

class Packer {
public:
  Packer() : top_cell_(nullptr) {}

  void load_rule_lib(const string &fn, Design *d) { prule_lib_.load(fn, d); }
  void load_config_lib(const string &fg) // added by hz
  {
    config_info_.load(fg);
  }

  void try_pack(Design *, bool);

  struct VCellDegGreater {
    bool operator()(const VCell *lhs, const VCell *rhs) const {
      if (lhs->degree() > rhs->degree())
        return true;
      else if (lhs->degree() < rhs->degree())
        return false;

      if (lhs->conn_factor() < rhs->conn_factor())
        return true;
      else if (lhs->conn_factor() > rhs->conn_factor())
        return false;
      return false; // lhs > rhs;
    }
  };

  struct VCellGainGreater {
    bool operator()(const VCell *lhs, const VCell *rhs) const {
      if (lhs->total_gain() > rhs->total_gain())
        return true;
      else if (lhs->total_gain() < rhs->total_gain())
        return false;

      return false; // lhs > rhs;
    }
  };

  typedef unique_list<VCell *, VCellDegGreater> VCellList;

  void write_xdl(Design *, bool);
  void block_config();

private:
  void map_to_hw_lib();
  void preprocess_macro();
  void map_to_vcell();
  void map_to_cluster();
  void map_to_slice();
  void map_to_io();
  void map_to_dll();
  void map_to_tbuf();
  void map_finish();
  void add_to_cluster(VCell *);
  void update_partial_gain(VPort *);
  VCell *get_next_vcell();
  void set_vcell_clusterd(VCell *);
  void reset_clustering_structs();
  void remove_residues(Design *);

  void map_to_macro();
  void pack_bram();
  void pack_dram();
  void pack_multiplier();
  void pack_carry_chain();
  void pack_lut6();
  void pack_lut5();

private:
  PKCell *top_cell_;
  PackRuleLib prule_lib_;
  NetlistCSP ncsp;
  Transformer tf_;

  PtrVector<Match> matches_;

  // Clustering Member
  VCellList uncluster_vcells_;
  VCellList cluster_vcells_;
  std::vector<VCell *> mark_vcells_;
  std::vector<PKNet *> mark_nets_;

  ConfigInfo config_info_;
};

} // namespace PACK

#endif