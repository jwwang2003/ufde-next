#include <fstream>
#include <rapidxml/rapidxml_print.hpp>

#include "Match.h"
#include "xmlutils.h"

namespace {
using namespace PACK;
using namespace FDU::XML;

class MatchWriter : public DomBuilder {
public:
  MatchWriter(const Match *match) : DomBuilder() { write_matches(match); }
  void write_xml(std::ostream &os) const { os << document(); }

private:
  void write_matches(const Match *match);
  void write_amatch(const Match::InstMatch *amatch, xml_node *node);
};

void MatchWriter::write_matches(const Match *match) {
  xml_node *root = create_element(&document(), "matches");
  set_attribute(root, "rule", match->rule()->name());
  set_attribute(root, "image_cell", match->image_cell()->name());
  set_attribute(root, "num_match", match->matches().size());

  for (Match::InstMatch *amatch : match->matches())
    write_amatch(amatch, root);
}

void MatchWriter::write_amatch(const Match::InstMatch *amatch, xml_node *root) {
  xml_node *ematch = create_element(root, "match");
  for (const Match::InstPair &inst_pair : *amatch) {
    xml_node *epair = create_element(ematch, "pair");
    set_attribute(epair, "origin", inst_pair.rule_inst->name());
    set_attribute(epair, "image", inst_pair.image_inst->name());
  }
}
} // namespace

namespace PACK {

using namespace std;

//////////////////////////////////////////////////////////////////////////
// class VPin
void VPin::unhook() {
  if (net_) {
    net_->remove_vpin(this);
    net_ = nullptr;
  }
}
void VPin::hookup(PKNet *net) {
  ASSERT(net_ == nullptr, "vpin already hooked");
  net->add_vpin(this);
  net_ = net;
}

//////////////////////////////////////////////////////////////////////////
// class VCell
VCell::VCell(RuleCell *rc, Match::InstMatch *amatch)
    : co_rcell_(rc), amatch_(amatch) {}

VPort *VCell::find_port(const string &n) {
  for (VPort *p : ports_)
    if (p->name() == n)
      return p;
  return nullptr;
}

PKInstance *VCell::find_image_inst(const string &rinst_name) const {
  for (Match::InstPair &ipair : *amatch_) {
    if (ipair.rule_inst->name() == rinst_name)
      return ipair.image_inst;
  }
  return nullptr;
}

void VCell::compute_total_gain() {
  static const float alpha = 0.5;
  pk_fac_.total_gain =
      (1.0 - alpha) * pk_fac_.share_gain + alpha * pk_fac_.conn_gain;
}

void VCell::compute_seed_factor() {
  static vector<PKNet *> mark_net;

  mark_net.clear();
  for (VPort *vport : ports_) {
    PKNet *net = vport->vpin()->net();
    if (net != nullptr && !net->is_touch()) {
      ++pk_fac_.degree;
      net->set_touch_flag();
      mark_net.push_back(net);
    }
  }

  size_t num_terminals = 0;
  for (PKNet *net : mark_net) {
    num_terminals += net->num_vpins();
    net->clear_touch_flag();
  }

  pk_fac_.conn_factor =
      (float)num_terminals / (pk_fac_.degree * pk_fac_.degree);
}

//////////////////////////////////////////////////////////////////////////
// class Match
Match::Match(Rule *rule, PKCell *image_cell)
    : rule_(rule), image_cell_(image_cell) {
  if (rule_->rule_cell()->relative_graph() == nullptr) {
    rule_->rule_cell()->build_graph();
#if defined(_DEBUG) && defined(_DRAW)
    rule_->rule_cell().relative_graph()->draw();
#endif
  }

  if (image_cell_->relative_graph() == nullptr) {
    image_cell_->build_graph();
#if defined(_DEBUG) && defined(_DRAW)
    image_cell_->relative_graph()->draw();
#endif
  }
}

void Match::next_match() {
  InstMatch &amatch = *match_enumer_->ref_of_next_elem();
  for (InstPair &inst_pair : amatch)
    inst_pair.rule_inst->set_image(inst_pair.image_inst);
}

void Match::make_vcell() {
  if (rport_pin_map.empty()) {
    for (RulePort *rport : rule_->rule_cell()->ports()) {
      Net *inner_net = rport->mpin()->net();
      if (inner_net) {
        for (Pin *pin : inner_net->pins())
          if (!pin->is_mpin()) {
            rport_pin_map[rport] = pin;
            break;
          }
        //					FDU_LOG(INFO) << "************ "
        //<< inner_net->name();
      }
      //				else
      //					rport_pin_map[rport] = nullptr;
    }
  }

  if (vcells_.size() == 0) {
    for (InstMatch *amatch : matches_) {
      VCell *vcell = vcells_.add(new VCell(rule_->rule_cell(), amatch));
      //				for (_RPortPinMap::value_type&
      // port_pin_pair: rport_pin_map) {
      for (RulePort *rport : rule_->rule_cell()->ports()) {
        VPort *vport = vcell->create_port(rport);
        Pin *rpin = rport_pin_map[rport]; // port_pin_pair.second
        if (rpin != nullptr) {
          PKInstance *image_inst =
              vcell->find_image_inst(rpin->instance()->name());
          Pin *real_pin = image_inst->find_pin(rpin->name());
          if (real_pin != nullptr && real_pin->net() != nullptr)
            vport->vpin()->hookup(static_cast<PKNet *>(real_pin->net()));
        }
      }
    }
  }
}

void Match::write() const {
  string fn = "match_result\\" + rule_->name() + ".xml";
  ofstream o(fn.c_str());
  MatchWriter(this).write_xml(o);
}
} // namespace PACK
