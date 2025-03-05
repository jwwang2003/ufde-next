#include "PKApp.h"

namespace PACK {

using namespace std;
using namespace VCELL_ATTR;
using namespace CHIP_TYPE_NAME;

void Packer::try_pack(Design *design, bool fEncry_) {
  // initialize target objects
  top_cell_ =
      static_cast<PKCell *>(design->top_module()); // PKCell��moduleΪ����
  tf_.set_top_cell(top_cell_);                     // tfΪtransformer�����

  // first remove all dangling elements, need to confirm
  tf_.remove_dangling();

  // two steps for csp packing
  // cout <<InfoMsg(InfoMsg::INFO_MAP2HW) << endl;
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_MAP2HW);
  map_to_hw_lib(); // ӳ�䵽device_lib

  // cout <<InfoMsg(InfoMsg::INFO_PREMACRO) << endl;
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_PREMACRO);
  preprocess_macro(); // ӳ�䵽macro_lib

  // cout <<InfoMsg(InfoMsg::INFO_MAP2MACRO) << endl;
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_MAP2MACRO);
  map_to_vcell(); // ӳ�䵽normal_lib
  map_to_macro();

  // cout <<InfoMsg(InfoMsg::INFO_MAP2CLUSTER) << endl;
  FDU_LOG(INFO) << InfoMsg(InfoMsg::INFO_MAP2CLUSTER);
  map_to_slice();
  map_to_io();
  // map_to_cluster();
  map_to_dll();
  map_to_tbuf();
  map_finish();

  // remove_residues(design);
  /*if (need_xdl_){
          write_xdl(design, fEncry_);
  }*/
  if (!PKApp::instance().xdl_fname().empty())
    write_xdl(design, fEncry_);
}

void Packer::map_to_hw_lib() {
  top_cell_
      ->enable_update_graph(); // top_cell_ΪPKCell�ָ࣬����ƵĶ����ģ��Module��

  RuleLibaray *hw_rule_lib = prule_lib_.hw_rule_lib();
  ASSERT(hw_rule_lib, "hardware rule library not found");

  for (RuleLibaray::Layer *layer : hw_rule_lib->layers()) { // ����ÿ��layer
    if (layer->empty())
      continue;
    PtrVector<Match> layer_matches;
    for (Rule *rule : *layer) { // ��ÿ��rule�Ͷ���ģ�鹹��һ��match
      Match *amatch =
          layer_matches.add(new Match(rule, top_cell_)); // Match(Rule*,PKCell)
      ncsp.solve(amatch); // �����ɵ�match����CSPƥ��
#ifdef _DEBUG
      if (amatch->num_matches() != 0)
        amatch->write(); // debugģʽ�����ƥ��������xml��ʽ��
#endif
    }
    // add to transform here!
    for (Match *amatch : layer_matches) {
      tf_.transform(amatch);
    }
    tf_.remove_dangling(true);
#ifdef _DEBUG
    // top_cell_->relative_graph()->draw();
#endif
    top_cell_->relative_graph()->clear_rule_lock();
    top_cell_->relative_graph()->clear_layer_lock();
  }

  top_cell_->stop_update_graph();
#ifdef _DEBUG
  // PKApp::instance().design().save(fileio::verilog,
  //	PKApp::instance().result_fname());
#endif
}

void Packer::preprocess_macro() {
  top_cell_->enable_update_graph();

  RuleLibaray *macro_rule_lib = prule_lib_.macro_rule_lib();
  ASSERT(macro_rule_lib, "macro rule library not found");

  for (RuleLibaray::Layer *layer : macro_rule_lib->layers()) {
    if (layer->empty())
      continue;
    PtrVector<Match> layer_matches;
    for (Rule *rule : *layer) {
      Match *amatch = layer_matches.add(new Match(rule, top_cell_));
      ncsp.solve(amatch);
#ifdef _DEBUG
      if (amatch->num_matches() != 0)
        amatch->write();
#endif
    }

    // add to transform here!
    for (Match *amatch : layer_matches) {
      tf_.transform(amatch);
    }
    tf_.remove_dangling(true);

#ifdef _DEBUG
    // top_cell_->relative_graph()->draw();
#endif
    top_cell_->relative_graph()->clear_rule_lock();
    top_cell_->relative_graph()->clear_layer_lock();
  }

  top_cell_->stop_update_graph();
#ifdef _DEBUG
  // PKApp::instance().design().save(fileio::verilog,
  //	PKApp::instance().result_fname());
#endif
}

void Packer::map_to_vcell() {
  RuleLibaray *pk_rule_lib = prule_lib_.pack_rule_lib();
  ASSERT(pk_rule_lib, "pack rule library not found");

  for (RuleLibaray::Layer *layer : pk_rule_lib->layers()) {
    if (layer->empty())
      continue;
    for (Rule *rule : *layer) {
      Match *amatch = matches_.add(new Match(rule, top_cell_));
      ncsp.solve(amatch);
      amatch->make_vcell();
#ifdef _DEBUG
      if (amatch->num_matches() != 0)
        amatch->write();
#endif
    }
  }

  for (Match *amatch : matches_) {
    for (VCell *vcell : amatch->vcells()) {
      vcell->compute_seed_factor();
      uncluster_vcells_.insert(vcell);
    }
  }
}

void Packer::map_to_macro() {
  pack_bram();
  pack_dram();
  pack_multiplier();
  pack_carry_chain();
  pack_lut6();
  pack_lut5();
}

void Packer::map_to_slice() {
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::SLICE) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      // PKInstance* slice = tf_.slice_inst();
      // slice->set_property<string>(CFG_NAME::CFG_TYPE, CFG_NAME::SLICE);
      set_vcell_clusterd(vc);
    }
  }
}

void Packer::map_to_dll() {
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::DLL) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      // PKInstance* slice = tf_.slice_inst();
      // slice->set_property<string>(CFG_NAME::CFG_TYPE, CFG_NAME::SLICE);
      set_vcell_clusterd(vc);
    }
  }
}

void Packer::map_to_tbuf() {
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::TBUF) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      // PKInstance* slice = tf_.slice_inst();
      // slice->set_property<string>(CFG_NAME::CFG_TYPE, CFG_NAME::SLICE);
      set_vcell_clusterd(vc);
    }
  }
}

void Packer::map_to_io() {
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::IOB) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      // PKInstance* iob = tf_.slice_inst();
      // iob->set_property<string>(CFG_NAME::CFG_TYPE, CFG_NAME::IOB);
      set_vcell_clusterd(vc);
    }
  }

  uncluster_loop = uncluster_vcells_;
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::GCLKIOB ||
        vc->type() == VCELL_TYPE::BUFGMUX) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      // PKInstance* gclkiob = tf_.slice_inst();
      // gclkiob->set_property<string>(CFG_NAME::CFG_TYPE, CFG_NAME::GCLKIOB);
      set_vcell_clusterd(vc);
    }
  }
}

void Packer::map_finish() {
  ASSERT(uncluster_vcells_.empty(), "There exists vcell unclusterd!");

  tf_.remove_dangling(false);

  for (PKInstance *inst : top_cell_->instances()) {
    string tcfg = inst->module_type();
    // mark the clock net
    if (tcfg == CFG_NAME::GCLK || tcfg == CFG_NAME::BUFGMUX) {
      for (PKPin *pin : inst->pins())
        if (pin->is_source())
          pin->net()->set_type(NetType::CLOCK);
    }
  }
}

void Packer::map_to_cluster() {
  RuleLibaray *pk_rule_lib = prule_lib_.pack_rule_lib();
  ASSERT(pk_rule_lib, "pack rule library not found");

  list<VCell *> uncluster_vcells_loop(uncluster_vcells_);
  for (VCell *seed_vcell : uncluster_vcells_loop) {
    if (seed_vcell->is_clustered())
      continue;

    if (seed_vcell->rule()->layer_idx() != SLICE_LAYER) {
      add_to_cluster(seed_vcell);
      continue;
    }

    tf_.set_sfull_flag();
    add_to_cluster(seed_vcell);
    tf_.clear_sfull_flag();

    VCell *next_vcell = get_next_vcell();
    ////////////////////////sophie/////////////////////////////////
    while (next_vcell != nullptr && !next_vcell->is_clustered()) {
      add_to_cluster(next_vcell);
      next_vcell = get_next_vcell();
    }
    reset_clustering_structs();
  }
}

void Packer::add_to_cluster(VCell *vcell) {
  set_vcell_clusterd(vcell);
  tf_.transform(vcell);

  if (vcell->rule()->layer_idx() != SLICE_LAYER)
    return;

  for (VPort *vport : vcell->vports())
    update_partial_gain(vport);

  for (VCell *vcell : mark_vcells_)
    vcell->compute_total_gain();
}

void Packer::update_partial_gain(VPort *vport) {
  PKNet *net = vport->vpin()->net();
  if (net == nullptr)
    return;

  // update share gain
  if (!net->is_touch()) {
    mark_nets_.push_back(net);
    net->set_touch_flag();

    for (VCell *vcell : net->unique_vcells()) {
      if (vcell->is_clustered())
        continue;

      vcell->inc_share_gain(1);
      if (!vcell->is_touch()) {
        mark_vcells_.push_back(vcell);
        vcell->set_touch_flag();
      }
    }
  }

  // update connection gain
  for (VPin *vpin : net->vpins()) {
    if (vpin->vport()->owner()->is_clustered())
      continue;
    if (vport->direction() == COS::OUTPUT ||
        vpin->vport()->direction() == COS::OUTPUT)
      vpin->vport()->owner()->inc_conn_gain(1);
  }
}

VCell *Packer::get_next_vcell() {
  unique_list<VCell *, VCellGainGreater> candidate_vcells;

  for (VCell *vc : mark_vcells_)
    candidate_vcells.insert(vc);
  // sort(mark_vcells_.begin(), mark_vcells_.end(), VCellGainGreater);
  for (VCell *cddt_vcell : candidate_vcells) {
    if (cddt_vcell->rule()->layer_idx() != SLICE_LAYER)
      continue;
    if (tf_.is_feasible(cddt_vcell))
      return cddt_vcell;
  }
  return nullptr;
}

void Packer::reset_clustering_structs() {
  for (PKNet *net : mark_nets_)
    net->clear_touch_flag();
  for (VCell *vcell : mark_vcells_) {
    vcell->clear_touch_flag();
    vcell->reset_share_gain();
    vcell->reset_conn_gain();
  }
}
/////////////////////sophie///////////////
void Packer::remove_residues(Design *design) {
  unique_list<PKInstance *> inst_residues_;
  unique_list<PKNet *> net_residues_;

  Library *work_lib = design->work_lib();
  for (Module *cell : work_lib->modules()) {
    if (cell->path_name() == design->top_module()->path_name())
      continue;

    for (Instance *inst : cell->instances()) {
      PKInstance *pkinst = static_cast<PKInstance *>(inst);
      if (!pkinst->is_used()) {
        for (PKPin *pin : pkinst->pins()) {
          PKNet *net = pin->net();
          ASSERT(net, "net inside clusters must exist");
          if (!net->is_used())
            // net->release();
            net_residues_.insert(net);
        }
        // pkinst->release();
        inst_residues_.insert(pkinst);
      }
    }
  }
  for (PKInstance *pkinst : inst_residues_)
    pkinst->release();
  for (PKNet *pknet : net_residues_)
    pknet->release();
}

void Packer::set_vcell_clusterd(VCell *vc) {
  vc->set_clustered();
  uncluster_vcells_.erase(vc);
  cluster_vcells_.insert(vc);
}

void Packer::block_config() {
  // config_info_.init();
}

} // namespace PACK