/*! \file Match.h
 *  \brief  Header of match result
 *  \author S.C. Leung
 *  \date   2009.05 - 2009.06
 *
 */

#ifndef _MATCH_H
#define _MATCH_H

#include "CellGraph.h"

namespace PACK {

class VCell;
class VPort;

class VPin {
public:
  VPin(VPort *vport) : vport_(vport), net_(nullptr) {}

  VPort *vport() { return vport_; }
  const VPort *vport() const { return vport_; }

  PKNet *net() const { return net_; }
  void rehook(PKNet *net) {
    unhook();
    hookup(net);
  }
  void unhook();
  void hookup(PKNet *net);

private:
  VPort *vport_;
  PKNet *net_;
};

class VPort {
public:
  typedef VCell owner_type;

  VPort(const RulePort *rport, owner_type *owner)
      : vpin_(this), co_port_(rport), owner_(owner) {}

  owner_type *owner() const { return owner_; }
  const VPin *vpin() const { return &vpin_; }
  const string &name() const { return co_port_->name(); }
  DirType direction() const { return co_port_->dir(); }
  VPin *vpin() { return &vpin_; }

private:
  VPin vpin_;
  const RulePort *co_port_;
  owner_type *owner_;
};

class Match {
public:
  struct InstPair {
    RuleInstance *rule_inst;
    PKInstance *image_inst;

    InstPair(RuleInstance *r = nullptr, PKInstance *i = nullptr)
        : rule_inst(r), image_inst(i) {}
  };

  typedef std::vector<InstPair> InstMatch;
  typedef Enumeration<PtrVector<InstMatch>::iterator> MatchEnumer;
  typedef PtrVector<InstMatch>::const_range_type const_matches_type;
  typedef PtrVector<InstMatch>::range_type matches_type;

  Match(Rule *rule, PKCell *image_cell);

  Rule *rule() const { return rule_; }
  PKCell *image_cell() const { return image_cell_; }
  auto num_matches() const { return matches_.size(); }
  const_matches_type matches() const { return matches_.range(); }
  matches_type matches() { return matches_.range(); }

  InstMatch *create_amatch() { return matches_.add(new InstMatch); }
  void load_matches() {
    match_enumer_.reset(new MatchEnumer(matches_.begin(), matches_.end()));
  }
  bool has_more_match() { return match_enumer_->has_more_elem(); }
  void next_match();

  // make VCell
  typedef PtrVector<VCell>::const_range_type const_vcells_type;
  typedef PtrVector<VCell>::range_type vcells_type;

  const_vcells_type vcells() const;
  vcells_type vcells();
  void make_vcell();

  // debug interface
  void write() const;

private:
  Rule *rule_;
  PKCell *image_cell_;
  PtrVector<InstMatch> matches_;
  boost::scoped_ptr<MatchEnumer> match_enumer_;

  typedef std::map<RulePort *, Pin *> _RPortPinMap;
  _RPortPinMap rport_pin_map;
  PtrVector<VCell> vcells_;
};

class VCell {
  friend class Match;
  struct PKFactor {
    bool is_clustered;
    bool touch_flag;

    int degree;
    float conn_factor;

    int share_gain;
    int conn_gain;
    float total_gain;

    PKFactor()
        : is_clustered(false), touch_flag(false), degree(0), conn_factor(0.),
          share_gain(0), conn_gain(0), total_gain(0.) {}
  };

public:
  VCell(RuleCell *rc, Match::InstMatch *amatch);

  Rule *rule() const { return co_rcell_->rule(); }
  VCELL_ATTR::VCELL_TYPE type() const {
    return static_cast<VCELL_ATTR::VCELL_TYPE>(rule()->layer_idx());
  }

  typedef boost::iterator_range<Match::InstMatch::const_iterator>
      const_instpairs_type;
  typedef boost::iterator_range<Match::InstMatch::iterator> instpairs_type;

  const_instpairs_type inst_pairs() const {
    return const_instpairs_type(amatch_->begin(), amatch_->end());
  }
  instpairs_type inst_pairs() {
    return instpairs_type(amatch_->begin(), amatch_->end());
  }
  PKInstance *find_image_inst(const string &) const;

  typedef PtrVector<VPort>::const_range_type const_ports_type;
  typedef PtrVector<VPort>::range_type ports_type;

  const_ports_type vports() const { return ports_.range(); }
  ports_type vports() { return ports_.range(); }
  VPort *find_port(const string &n);

  // Clustering Interface
  bool is_touch() const { return pk_fac_.touch_flag; }
  bool is_clustered() const { return pk_fac_.is_clustered; }
  int degree() const { return pk_fac_.degree; }
  float conn_factor() const { return pk_fac_.conn_factor; }
  float total_gain() const { return pk_fac_.total_gain; }

  void set_touch_flag() { pk_fac_.touch_flag = true; }
  void clear_touch_flag() { pk_fac_.touch_flag = false; }
  void set_clustered() { pk_fac_.is_clustered = true; }
  void inc_share_gain(int i) { pk_fac_.share_gain += i; }
  void inc_conn_gain(int i) { pk_fac_.conn_gain += i; }
  void reset_share_gain() { pk_fac_.share_gain = 0; }
  void reset_conn_gain() { pk_fac_.conn_gain = 0; }

  void compute_total_gain();
  void compute_seed_factor();

private:
  VPort *create_port(const RulePort *rport) {
    return ports_.add(new VPort(rport, this));
  }

private:
  Match::InstMatch *amatch_;
  PtrVector<VPort> ports_;
  RuleCell *co_rcell_;

  // Clustering Factors
  PKFactor pk_fac_;
};

inline Match::const_vcells_type Match::vcells() const {
  return vcells_.range();
}
inline Match::vcells_type Match::vcells() { return vcells_.range(); }

} // namespace PACK

#endif