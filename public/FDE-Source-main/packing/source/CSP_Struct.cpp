#include "CSP_Struct.h"

namespace PACK {

void Variable::add_incoming_query(Query *q) {
  in_queries_.push_back(q);
  if (q->qtype() == Query::QUERY_TYPE)
    type_query_ = static_cast<QueryType *>(q);
}

void Variable::check_constraints(std::vector<Variable *> &conflict_vars) {
  for (BConstraint *c : constraints_)
    if (c->appliable() && !c->execute())
      conflict_vars.push_back(c->get_cause(this));
}

BConstraint::BConstraint(Variable *var, int weight, ConstraintEnum type)
    : var1_(var), var2_(var), weight_(weight), active_(true), type_(type) {
  var1_->add_constraint(this);
  ++weight_;
}

BConstraint::BConstraint(Variable *var1, Variable *var2, int weight,
                         ConstraintEnum type)
    : var1_(var1), var2_(var2), weight_(weight), active_(true), type_(type) {
  var1_->add_constraint(this);
  var2_->add_constraint(this);
  weight_ += 2;
}

ConstraintType::ConstraintType(Variable *var)
    : BConstraint(var, 0, BConstraint::CONSTRAINT_TYPE) {}

ConstraintSource::ConstraintSource(Variable *src, Variable *arc)
    : BConstraint(src, arc, 0, BConstraint::CONSTRAINT_SOURCE) {}

ConstraintTarget::ConstraintTarget(Variable *tar, Variable *arc)
    : BConstraint(tar, arc, 0, BConstraint::CONSTRAINT_TARGET) {}

ConstraintComNet::ConstraintComNet(Variable *var1, Variable *var2)
    : BConstraint(var1, var2, 0, BConstraint::CONSTRAINT_COMNET) {}

Query::Query(Variable *tar, int weight, BConstraint *c, QueryEnum e)
    : source_(nullptr), target_(tar), weight_(weight), correspond_(c), qtype_(e) {
  tar->add_incoming_query(this);
}

Query::Query(Variable *src, Variable *tar, int weight, BConstraint *c,
             QueryEnum e)
    : source_(src), target_(tar), weight_(weight), correspond_(c), qtype_(e) {
  src->add_outgoing_query(this);
  tar->add_incoming_query(this);
}

QueryType::QueryType(Variable *var, BConstraint *c)
    : Query(var, 0, c, Query::QUERY_TYPE), average_in_(-1), average_out_(-1) {}

QuerySource::QuerySource(Variable *arc, Variable *tar, BConstraint *c)
    : Query(arc, tar, 0, c, Query::QUERY_SRC) {}

QueryTarget::QueryTarget(Variable *arc, Variable *tar, BConstraint *c)
    : Query(arc, tar, 0, c, Query::QUERY_TAR) {}

QueryIncoming::QueryIncoming(Variable *src, Variable *arc, BConstraint *c)
    : Query(src, arc, 3, c, Query::QUERY_IN) {}

QueryOutgoing::QueryOutgoing(Variable *src, Variable *arc, BConstraint *c)
    : Query(src, arc, 6, c, Query::QUERY_OUT) {}

// QueryComSrc::QueryComSrc(Variable* src, Variable* tar, BConstraint* c)
//	: Query(src, tar, 2, c) {}

//////////////////////////////////////////////////////////////////////////
//  members of Constraints
bool BConstraint::appliable() const {
  if (!active_)
    return false;
  return var1_->instance() != nullptr && var2_->instance() != nullptr;
}

bool ConstraintType::execute() {
  GObject *rule_obj = var1_->rule_gobj();
  GObject *inst_obj = var1_->instance();

  if (rule_obj->type() != inst_obj->type())
    return false;

  if (rule_obj->kind() == GObjType::GNODE) {
    GNode *rule_node = static_cast<GNode *>(rule_obj);
    GNode *inst_node = static_cast<GNode *>(inst_obj);
    for (cstr_ptr cpstr : rule_node->connect_pin()) {
      Pin *pin = inst_node->correspond()->find_pin(*cpstr);
      if (pin == nullptr || pin->net() == nullptr)
        return false;
    }
  } else {
    GArc *rule_arc = static_cast<GArc *>(rule_obj);
    GArc *inst_arc = static_cast<GArc *>(inst_obj);

    if (rule_arc->sink_num_type() == GArcType::EQUAL &&
        rule_arc->num_pure_sink_of_net() != inst_arc->num_pure_sink_of_net())
      return false;

    if (rule_arc->sink_num_type() == GArcType::MORE &&
        rule_arc->num_pure_sink_of_net() >= inst_arc->num_pure_sink_of_net())
      return false;

    if (rule_arc->sink_num_type() == GArcType::NOT_LESS &&
        rule_arc->num_pure_sink_of_net() > inst_arc->num_pure_sink_of_net())
      return false;

    if (rule_arc->src_pin_name() != inst_arc->src_pin_name())
      return false;

    for (cstr_ptr rule_pin_name : rule_arc->sink_pin_names()) {
      bool found = false;
      for (cstr_ptr inst_pin_name : inst_arc->sink_pin_names())
        if (*rule_pin_name == *inst_pin_name) {
          found = true;
          break;
        }
      if (!found)
        return false;
    }
  }
  return true;
}

bool ConstraintSource::execute() {
  if (var1_->instance()->kind() != GObjType::GNODE ||
      var2_->instance()->kind() != GObjType::GARC)
    return false;

  if (var1_->instance() != static_cast<GArc *>(var2_->instance())->source())
    return false;

  return true;
}

bool ConstraintTarget::execute() {
  if (var1_->instance()->kind() != GObjType::GNODE ||
      var2_->instance()->kind() != GObjType::GARC)
    return false;

  if (var1_->instance() != static_cast<GArc *>(var2_->instance())->target())
    return false;

  return true;
}

bool ConstraintComNet::execute() {
  GNode *src_rule_node = static_cast<GNode *>(var1_->rule_gobj());
  GNode *tar_rule_node = static_cast<GNode *>(var2_->rule_gobj());
  const GNode::ComNetPairs &com_net_pairs =
      src_rule_node->get_common_net(tar_rule_node);

  PKInstance *src_inst = static_cast<GNode *>(var1_->instance())->correspond();
  PKInstance *tar_inst = static_cast<GNode *>(var2_->instance())->correspond();

  for (const GNode::PinNamesPair &apair : com_net_pairs) {
    Pin *apin = src_inst->find_pin(*apair.first[0]);
    if (apin == nullptr)
      return false;

    Net *com_net = apin->net();
    for (cstr_ptr cpstr : apair.first) {
      apin = src_inst->find_pin(*cpstr);

      if (apin == nullptr || apin->net() != com_net)
        return false;
    }

    for (cstr_ptr cpstr : apair.second) {
      apin = tar_inst->find_pin(*cpstr);

      if (apin == nullptr || apin->net() != com_net)
        return false;
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////
//  members of Query

// the appliable conditions are: first the target var is not instantiated
// second the source var must be instantiated
bool Query::appliable() const {
  if (target_->instance() != nullptr)
    return false;
  if (source_ != nullptr && source_->instance() == nullptr)
    return false;
  return true;
}

void QueryType::query_gobjects(CellGraph::GObjList *type_domain) {
  query_result_.clear();
  if (type_domain == nullptr)
    return;

  correspond_->activate();

  for (GObject *gobj : *type_domain) {
    target_->set_instance(gobj);
    if (!gobj->layer_lock() && correspond_->execute())
      query_result_.insert(gobj);
  }
  target_->set_instance(nullptr); //???
}

int QueryType::average_in_degree() {
  if (query_result_.empty())
    return 0;
  if (target_->rule_gobj()->kind() == GObjType::GARC)
    return 0;

  if (average_in_ == -1) {
    int in = 0;
    for (GObject *gobj : query_result_)
      in += static_cast<GNode *>(gobj)->num_incoming_arcs();

    average_in_ = in / query_result_.size();
  }
  return average_in_;
}

int QueryType::average_out_degree() {
  if (query_result_.empty())
    return 0;
  if (target_->rule_gobj()->kind() == GObjType::GARC)
    return 0;

  if (average_out_ == -1) {
    int out = 0;
    for (GObject *gobj : query_result_)
      out += static_cast<GNode *>(gobj)->num_outgoing_arcs();

    average_out_ = out / query_result_.size();
  }
  return average_out_;
}

int QueryType::size() { return query_result_.size(); }

int QuerySource::size() { return 1; }

int QueryTarget::size() { return 1; }

int QueryIncoming::size() { return source_->type_query()->average_in_degree(); }

int QueryOutgoing::size() {
  return source_->type_query()->average_out_degree();
}

Variable::DomEnumer QueryType::execute() {
  return Variable::DomEnumer(query_result_.begin(), query_result_.end());
}

Variable::DomEnumer QuerySource::execute() {
  query_result_.clear();
  query_result_.insert(static_cast<GArc *>(source_->instance())->source());
  return Variable::DomEnumer(query_result_.begin(), query_result_.end());
}

Variable::DomEnumer QueryTarget::execute() {
  query_result_.clear();
  query_result_.insert(static_cast<GArc *>(source_->instance())->target());
  return Variable::DomEnumer(query_result_.begin(), query_result_.end());
}

Variable::DomEnumer QueryIncoming::execute() {
  query_result_.clear();
  GNode *gnode = static_cast<GNode *>(source_->instance());
  for (GArc *in_arc : gnode->incoming_arcs())
    if (target_->type_query()->type_ok(in_arc))
      query_result_.insert(in_arc);
  return Variable::DomEnumer(query_result_.begin(), query_result_.end());
}

Variable::DomEnumer QueryOutgoing::execute() {
  query_result_.clear();
  GNode *gnode = static_cast<GNode *>(source_->instance());
  for (GArc *out_arc : gnode->outgoing_arcs())
    if (target_->type_query()->type_ok(out_arc))
      query_result_.insert(out_arc);
  return Variable::DomEnumer(query_result_.begin(), query_result_.end());
}
} // namespace PACK
