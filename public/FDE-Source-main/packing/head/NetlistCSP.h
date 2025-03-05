/*! \file NetlistCSP.h
 *  \brief  Header of CSP solver
 *  \author S.C. Leung
 *  \date   2009.05 - 2009.06
 *
 */

#ifndef _NETLIST_CSP_H
#define _NETLIST_CSP_H

#include <boost/ptr_container/ptr_map.hpp>
#include <functional>
#include <map>
#include <utility>

#include "CSP_Struct.h"
#include "Match.h"

namespace PACK {

class NetlistCSP;

class SolutionBackjump {
public:
  SolutionBackjump(NetlistCSP *);
  void solve();

private:
  // function object
  struct VarSizeLess {
    bool operator()(const Variable *lhs, const Variable *rhs) const {
      if (lhs->type_query()->size() < rhs->type_query()->size())
        return true;
      else if (lhs->type_query()->size() > rhs->type_query()->size())
        return false;
      return false; // lhs < rhs;
    }
  };

  struct VarQOrderLess {
    bool operator()(const Variable *lhs, const Variable *rhs) const {
      if (lhs->query_order() < rhs->query_order())
        return true;
      else if (lhs->query_order() > rhs->query_order())
        return false;
      return false; // lhs < rhs;
    }
  };

  typedef unique_list<Variable *, VarSizeLess> SizeOrderVarSet;
  typedef unique_list<Variable *, VarQOrderLess> QOrderVarSet;

private:
  void build_search_strategy();
  void search_breadth_first(SizeOrderVarSet &, SizeOrderVarSet &, GObject *);
  Query *get_best_query(Variable *);
  void get_reached_var(Variable *, const SizeOrderVarSet &, SizeOrderVarSet &);
  void add_backjump_target(Query *, QOrderVarSet &);
  Variable *get_top_conflict_var(const std::vector<Variable *> &);

  void add_injection(Variable *var) {
    injection_map_.insert(std::make_pair(var->instance(), var));
  }
  void remove_injection(Variable *var) {
    std::map<GObject *, Variable *>::iterator fit =
        injection_map_.find(var->instance());
    if (fit != injection_map_.end())
      injection_map_.erase(fit);
  }

  void clear();

private:
  NetlistCSP *ncsp_;
  std::vector<Query *> sorted_queries_;
  std::map<GObject *, Variable *> injection_map_;

private:
  static const int START = 1;
  static const int NEXT = 2;
  static const int INSTANTIATE = 3;
  static const int BACK = 4;
  static const int SUCCESS = 5;
  static const int NO_MORE_SOLUTION = 6;
  static const int BACK_JUMP = 7;
};

class NetlistCSP {
  friend class SolutionBackjump;

public:
  typedef std::map<GObject *, Variable *> GObjVarMap;

public:
  NetlistCSP();
  ~NetlistCSP();

  void solve(Match *);

private:
  void clear();
  void build_constraint_graph();
  void set_rule_graph(CellGraph *);
  void set_domain(CellGraph *);

  template <typename C> inline C *create_constraint(Variable *);
  template <typename C> inline C *create_constraint(Variable *, Variable *);

  template <typename Q> inline Q *create_query(Variable *, BConstraint *);
  template <typename Q>
  inline Q *create_query(Variable *, Variable *, BConstraint *);

  Variable *save_amatch();

private:
  CellGraph *rule_graph_;
  CellGraph *image_graph_;
  GObjVarMap obj_var_map_;
  std::set<string> type_set_;
  PtrVector<Query> queries_;
  PtrVector<BConstraint> constraints_;
  SolutionBackjump solver_;

  Match *cur_match_;
};

template <typename C> inline C *NetlistCSP::create_constraint(Variable *v) {
  C *c = new C(v);
  constraints_.push_back(c);
  return c;
}

template <typename C>
inline C *NetlistCSP::create_constraint(Variable *v1, Variable *v2) {
  C *c = new C(v1, v2);
  constraints_.push_back(c);
  return c;
}

template <typename Q>
inline Q *NetlistCSP::create_query(Variable *v, BConstraint *c) {
  Q *q = new Q(v, c);
  queries_.push_back(q);
  return q;
}

template <typename Q>
inline Q *NetlistCSP::create_query(Variable *v1, Variable *v2, BConstraint *c) {
  Q *q = new Q(v1, v2, c);
  queries_.push_back(q);
  return q;
}
} // namespace PACK

#endif