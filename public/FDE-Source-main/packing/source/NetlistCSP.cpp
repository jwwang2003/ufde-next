#include <boost/assign/ptr_map_inserter.hpp>

#include "NetlistCSP.h"

namespace PACK {

using namespace std;
using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////
//  NetlistCSP
NetlistCSP::NetlistCSP()
    : cur_match_(nullptr), rule_graph_(nullptr), image_graph_(nullptr), solver_(this) {}

NetlistCSP::~NetlistCSP() { clear(); }

void NetlistCSP::solve(Match *match) {
  cur_match_ = match;
  set_rule_graph(cur_match_->rule()
                     ->rule_cell()
                     ->relative_graph()); // ����ǰrule�µ�cell��Ϊ��ǰͼ
  set_domain(cur_match_->image_cell()->relative_graph());
  solver_.solve();
}

void NetlistCSP::set_rule_graph(CellGraph *rg) {
  if (rg == rule_graph_)
    return; // ���Ѿ���ø�rule��cellͼ���򷵻�

  clear();

  rule_graph_ = rg;
  build_constraint_graph(); // ������Լ����ͼ
}

void NetlistCSP::clear() {
  if (rule_graph_)
    for (GObject *gobj : rule_graph_->gobjects())
      delete obj_var_map_[gobj]; // ɾ��ͼ����ؽ��Ԫ�أ���ɾ����ͼ
  obj_var_map_.clear();
  type_set_.clear();
}

void NetlistCSP::build_constraint_graph() {
  Variable *var;
  for (GObject *gobj :
       rule_graph_->gobjects()) { // the gobjects are already in orders
    //			ptr_map_insert<Variable>(obj_var_map_)(gobj, gobj);
    obj_var_map_[gobj] = new Variable(gobj);
    if (type_set_.find(gobj->type()) == type_set_.end())
      type_set_.insert(gobj->type());
  }
  for (GObject *gobj : rule_graph_->gobjects()) {
    //		for (GObjVarMap::value_type& mt: obj_var_map_) {
    Variable *obj_var = obj_var_map_[gobj];

    BConstraint *constraint = create_constraint<ConstraintType>(obj_var);
    create_query<QueryType>(obj_var, constraint);

    if (gobj->kind() == GObjType::GARC) {
      GArc *garc = static_cast<GArc *>(gobj);
      Variable *src_var = obj_var_map_[garc->source()];
      Variable *tar_var = obj_var_map_[garc->target()];

      constraint = create_constraint<ConstraintSource>(src_var, obj_var);
      create_query<QuerySource>(obj_var, src_var, constraint);
      create_query<QueryOutgoing>(src_var, obj_var, constraint);

      constraint = create_constraint<ConstraintTarget>(tar_var, obj_var);
      create_query<QueryTarget>(obj_var, tar_var, constraint);
      create_query<QueryIncoming>(tar_var, obj_var, constraint);
    } else {
      GNode *gnode = static_cast<GNode *>(gobj);
      for (const GNode::CommonNet &com_net : gnode->common_nets()) {
        Variable *tar_var = obj_var_map_[com_net.target];
        create_constraint<ConstraintComNet>(obj_var, tar_var);
      }
    }
  }
}

void NetlistCSP::set_domain(CellGraph *ig) {
  image_graph_ = ig;
  // image_graph_->fill_type_map();
  for (GObject *gobj : rule_graph_->gobjects()) {
    //		for (GObjVarMap::value_type& mt: obj_var_map_) {
    QueryType *query = obj_var_map_[gobj]->type_query();
    query->query_gobjects(
        ig->find_gobjs(query->gobj_type())); // query results according to the
                                             // type name of the object

    // be careful about this call
    query->deactivate();
  }
}

Variable *NetlistCSP::save_amatch() {
  Variable *top_var_locked = nullptr;
  Match::InstMatch *amatch = cur_match_->create_amatch();
  for (GObject *rule_obj : rule_graph_->gobjects()) {
    //		for (GObjVarMap::value_type& mt: obj_var_map_) {
    Variable *rela_var = obj_var_map_[rule_obj];
    if (rule_obj->kind() == GObjType::GNODE) {
      RuleInstance *rule_inst = static_cast<RuleInstance *>(
          static_cast<GNode *>(rule_obj)->correspond());
      PKInstance *image_inst =
          static_cast<GNode *>(rela_var->instance())->correspond();
      amatch->push_back(Match::InstPair(rule_inst, image_inst));

      if (rule_obj->rule_to_lock()) {
        rela_var->instance()
            ->set_rule_lock(); // current result instance can be matched in the
                               // current rule only once
        if (top_var_locked == nullptr)
          top_var_locked = rela_var;
        else if (top_var_locked->query_order() > rela_var->query_order())
          top_var_locked = rela_var;
      }
      rela_var->instance()->set_layer_lock();
    }
  }

  return top_var_locked;
}

//////////////////////////////////////////////////////////////////////////
//  SolutionBackJump

SolutionBackjump::SolutionBackjump(NetlistCSP *ncsp) : ncsp_(ncsp) {}

void SolutionBackjump::solve() {
  clear();
  build_search_strategy();

  int status = START;
  int direction;
  int cur_query_idx;
  Query *cur_query;
  Variable *cur_var;

  QOrderVarSet backjump_vars;
  vector<Variable *> conflict_vars;
  Variable::DomEnumer *dom_enumer;

  Variable *top_var_locked;

  do {
    switch (status) {
    case START:
      cur_query_idx = -1;
      status = NEXT;
      break;

    // query variable's domain
    case NEXT:
      if (cur_query_idx >= (int)sorted_queries_.size() - 1) {
        status = SUCCESS;
      } else {
        cur_query = sorted_queries_[++cur_query_idx];
        cur_var = cur_query->tar_var();

        if (cur_query->appliable()) {
          cur_var->set_dom_enumer(
              cur_query->execute()); // instantiate the cur_var
          backjump_vars.clear();
          add_backjump_target(cur_query, backjump_vars); // cur_query's src_var
          status = INSTANTIATE;
        } else {
          status = NO_MORE_SOLUTION;
        }
      }
      direction = 2;
      break;

    case INSTANTIATE:
      if (direction == 2)
        status = BACK_JUMP;
      else
        status = BACK;

      dom_enumer = &cur_var->dom_enumer();
      cur_query->deactivate(); // deactivate correspondent constraint
      while (dom_enumer->has_more_elem()) {
        GObject *gobj = dom_enumer->val_of_next_elem();
        if (gobj->rule_lock())
          continue; // the gobj is image and unlocked

        map<GObject *, Variable *>::iterator fit = injection_map_.find(gobj);
        if (fit != injection_map_.end()) {
          if (status == BACK_JUMP)
            backjump_vars.insert(fit->second);
          continue;
        }

        cur_var->set_instance(gobj);
        conflict_vars.clear();
        cur_var->check_constraints(conflict_vars); //
        if (conflict_vars.size() == 0) {
          status = NEXT;
          add_injection(
              cur_var); // the pair of cur_var and its instantiated gobject
          break;
        }
        if (status == BACK_JUMP) {
          backjump_vars.insert(
              get_top_conflict_var(conflict_vars)); // conflict_vars do exist
        }
      }
      cur_query->activate();
      break;

    case BACK:
      if (cur_query_idx == 0) {
        if (cur_var->dom_enumer().has_more_elem()) {
          remove_injection(cur_var);
          cur_var->set_instance(nullptr);
          status = INSTANTIATE;
          direction = 2;
        } else {
          status = NO_MORE_SOLUTION;
          direction = 4;
        }
      } else if (cur_query_idx > 0) {
        remove_injection(cur_var);
        cur_var->set_instance(nullptr);
        cur_var = sorted_queries_[--cur_query_idx]->tar_var();
        remove_injection(cur_var);
        cur_var->set_instance(nullptr);
        status = INSTANTIATE;
        direction = 4;
      } else {
        status = NO_MORE_SOLUTION;
        direction = 4;
      }
      break;

    case BACK_JUMP:
      status = NO_MORE_SOLUTION;
      if (backjump_vars.size() > 0) {
        Variable *back_var = *(backjump_vars.rbegin());
        while (cur_query_idx > back_var->query_order()) {
          remove_injection(cur_var);
          cur_var->set_instance(nullptr);
          cur_var = sorted_queries_[--cur_query_idx]->tar_var();
        }
        remove_injection(cur_var);
        cur_var->set_instance(nullptr);
        cur_query = sorted_queries_[cur_query_idx];
        status = INSTANTIATE;
      }
      direction = 4;
      break;

    case SUCCESS:
      top_var_locked = ncsp_->save_amatch(); //???
      if (top_var_locked != nullptr)
        while (cur_query_idx > top_var_locked->query_order()) {
          remove_injection(cur_var);
          cur_var->set_instance(nullptr);
          cur_var = sorted_queries_[--cur_query_idx]->tar_var();
        }
      remove_injection(cur_var);
      cur_var->set_instance(nullptr);
      cur_query = sorted_queries_[cur_query_idx];
      status = INSTANTIATE;
      direction = 4;
      break;

    case NO_MORE_SOLUTION:
      return;
    }

  } while (true);
}

Variable *SolutionBackjump::get_top_conflict_var(
    const std::vector<Variable *> &conflict_vars) {
  Variable *top_var = conflict_vars[0];
  for (Variable *var : conflict_vars)
    if (top_var->query_order() > var->query_order())
      top_var = var;
  return top_var;
}

void SolutionBackjump::add_backjump_target(Query *q,
                                           QOrderVarSet &backjump_var) {
  if (q->src_var() != nullptr)
    backjump_var.insert(q->src_var());
}

void SolutionBackjump::build_search_strategy() {
  SizeOrderVarSet vars_left, breadth_vars;
  for (GObject *rule_obj : ncsp_->rule_graph_->gobjects())
    //		for (NetlistCSP::GObjVarMap::value_type& ov_pair:
    // ncsp_->obj_var_map_)
    vars_left.insert(
        ncsp_->obj_var_map_[rule_obj]); // set of vars to be instantiated

  GNode dummy(nullptr, "DUMMY", nullptr);
  search_breadth_first(breadth_vars, vars_left, &dummy); // �����������

  int query_order = 0;
  for (Query *q : sorted_queries_) {
    q->tar_var()->set_instance(nullptr); // thus is appliable
    q->tar_var()->set_query_order(query_order++);
  }
}
// search best query for each var using BFS
void SolutionBackjump::search_breadth_first(SizeOrderVarSet &breadth_vars,
                                            SizeOrderVarSet &vars_left,
                                            GObject *dummy) {
  if (!breadth_vars.empty()) {
    SizeOrderVarSet
        reached_var; // breadth searched vars related to the current var
    for (Variable *var : breadth_vars) {
      Query *best_query = get_best_query(var);
      if (best_query != nullptr)
        sorted_queries_.push_back(best_query);
      vars_left.erase(vars_left.find(var));
      var->set_instance(
          dummy); // some kind of flag indicating current var is instantiated
                  // and can be used for other query except query type

      get_reached_var(var, vars_left, reached_var);
    }
    search_breadth_first(reached_var, vars_left, dummy);
    return;
  }

  if (vars_left.empty())
    return;
  else {
    breadth_vars.insert(
        *vars_left.begin()); // var with the smallest query result size
    search_breadth_first(breadth_vars, vars_left, dummy);
  }
}

Query *SolutionBackjump::get_best_query(Variable *var) {
  int best_size = -1;
  Query *best_query = nullptr;
  bool found = false;

  for (Query *q : var->incoming_queries()) {
    if (q->appliable()) {
      if (!found) {
        best_size = q->size();
        best_query = q;
        found = true;
      } else if (best_size > q->size()) {
        best_size = q->size();
        best_query = q;
      }
    }
  }
  return best_query;
}

void SolutionBackjump::get_reached_var(Variable *var,
                                       const SizeOrderVarSet &vars_left,
                                       SizeOrderVarSet &reached_vars) {
  for (BConstraint *c : var->constraints()) {
    if (c->type() == BConstraint::CONSTRAINT_COMNET)
      continue;
    if (vars_left.find(c->get_cause(var)) !=
        vars_left.end()) // find the other var that caused the binary constraint
      reached_vars.insert(
          c->get_cause(var)); // next reachable vars to be queried
  }
}

void SolutionBackjump::clear() {
  injection_map_.clear();
  sorted_queries_.clear();
}

} // namespace PACK