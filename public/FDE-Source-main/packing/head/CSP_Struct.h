/*! \file   CSP_Struct.h
 *  \brief  Header of data structures about CSP
 *  \author S.C. Leung
 *  \date   2009.05 - 2009.06
 *  \attention To comprehend this file you should read through the paper
 *  "Utilizing Constraint Satisfaction Techniques for Efficient Graph Pattern
 * Matching".
 */

#ifndef _CSP_STRUCT_H
#define _CSP_STRUCT_H

#include "CellGraph.h"
#include "PKUtils.h"
#include <set>

namespace PACK {

// forward declaration
class Query;
class QueryType;
class BConstraint;

/*! \brief class %Variable
 *
 *  This class is used to represent a variable when solving match problem.
 *  A rule GNode or rule GArc will be converted to a %Variable.
 *  \sa NetlistCSP
 */
class Variable {
public:
  /*! \brief Java like domain enumerator
   *
   *  The possible values(image GObjects) of the %Variable form its domain,
   *  every value in domain must satisfy the type constraint of the
   * corresponding rule %GObject.
   */
  typedef Enumeration<unique_list<GObject *>::const_iterator> DomEnumer;

  /*! \brief constructor
   *
   *  \param robj The corresponding rule GObject
   */
  Variable(GObject *robj)
      : rule_gobj_(robj), type_query_(nullptr), query_order_(-1), instance_(nullptr) {
  }

  /*! \brief query order getter
   *
   *  We determine the value of the %Variable one by one until all Variables are
   * instantiated. Every %Variable have their query order to demonstrate when
   * they are instantiated. This order is determined by the search strategy of
   * the CSP solver. \return The query order of this %Variable
   */
  int query_order() const { return query_order_; }

  /*! \brief corresponding rule %GObject getter
   *
   *  \return The corresponding rule GObject
   */
  GObject *rule_gobj() const { return rule_gobj_; }

  /*! \brief instantiated %GObject
   *
   *  \return The instantiated GObject
   */
  GObject *instance() const { return instance_; }

  /*! \brief corresponding type query
   *
   *  A Query means we can use it to find out a %Variable's domain.
   *  A Query is executable when all its source Variables are instantiated.
   *  For example when we want to query the domain of the target %Variable,
   *  we must first instantiate the source %Variable and find out all target
   *  GObjects of this instance. QueryType require no source %Variable because
   *  it query the domain of the target %Variable according its rule GObject.
   *  \return The corresponding QueryType
   */
  QueryType *type_query() const { return type_query_; }

  /*! \brief domain enumerator getter
   *
   *  We return the reference of the enumerator because we want the enumerator
   *  remember where we have enumerated when doing search.
   *  \return The ref enumerator of the Variable's domain
   */
  DomEnumer &dom_enumer() { return dom_enumer_; }

  /*! \brief query order setter
   *
   *  \param order The query order
   */
  void set_query_order(int order) { query_order_ = order; }

  /*! \brief instance setter
   *
   *  \param o The instantiated GObject
   */
  void set_instance(GObject *o) { instance_ = o; }

  /*! \brief domain enumerator setter
   *
   *  The domain enumerator to set is return by Query::execute().
   *  \param de The domain enumerator to set
   */
  void set_dom_enumer(const DomEnumer &de) { dom_enumer_ = de; }

  /*! \brief add corresponding BConstraint
   *
   *  We use BConstraint to demonstrate what constraints the %Variable should be
   *  satisfied, 'B' for binary because all types of constraints are mutual.
   *  When instantiating a %Variable we will check if all the constraints are
   * satisfied. \param c The corresponding BConstraint to add
   */
  void add_constraint(BConstraint *c) { constraints_.push_back(c); }

  /*! \brief add outgoing query
   *
   *  One is outgoing Query of the %Variable if the %Variable is the source
   * %Variable of the Query. \param q The outgoing query to add
   */
  void add_outgoing_query(Query *q) { out_queries_.push_back(q); }

  /*! \brief add incoming query
   *
   *  One is incoming Query of the %Variable if the %Variable is the target
   * %Variable of the Query. This routine will set the type query of the
   * %Variable too. \param q The incoming query to add
   */
  void add_incoming_query(Query *q);

  /*! \brief iterator_range of %Query
   *
   */
  typedef boost::iterator_range<std::vector<Query *>::const_iterator>
      queries_type;

  /*! \brief incoming queries getter
   *
   *  \return The iterator_range of incoming queries
   */
  queries_type incoming_queries() const {
    return queries_type(in_queries_.begin(), in_queries_.end());
  }

  /*! \brief outgoing queries getter
   *
   *  \return The iterator_range of outgoing queries
   */
  queries_type outgoing_queries() const {
    return queries_type(out_queries_.begin(), out_queries_.end());
  }

  /*! \brief iterator_range of %BConstraint
   *
   */
  typedef boost::iterator_range<std::vector<BConstraint *>::const_iterator>
      csts_type;

  /*! \brief constraints getter
   *
   *  \return The iterator_range of BConstraint
   */
  csts_type constraints() const {
    return csts_type(constraints_.begin(), constraints_.end());
  }

  /*! \brief check all constraints
   *
   *  This routine checks if all the executable constraints are satisfied.
   *  A BConstraint is executable if the two corresponding Variables of the
   *  BConstraint are instantiated. This routine will be called when every
   *  %Variable is instantiated.
   *  \param[out] conflicts All conflict %Variable of this %Variable
   */
  void check_constraints(std::vector<Variable *> &conflicts);

private:
  /*! \brief corresponding rule %GObject
   *
   */
  GObject *rule_gobj_;

  /*! \brief all corresponding BConstraints of this %Variable
   *
   */
  std::vector<BConstraint *> constraints_;

  /*! \brief all incoming queries of this %Variable
   *
   */
  std::vector<Query *> in_queries_;

  /*! \brief all outgoing queries of this %Variable
   *
   */
  std::vector<Query *> out_queries_;

  /*! \brief corresponding type query
   *
   */
  QueryType *type_query_;

  /*! \brief query order
   *
   */
  int query_order_;

  /*! \brief domain enumerator
   *
   */
  DomEnumer dom_enumer_;

  /*! \brief instantiated %GObject
   *
   */
  GObject *instance_;
};

/*! \brief class %BConstraint
 *
 *  This class is used to represent the constraint that Variable should satisfy.
 */
class BConstraint {
public:
  enum ConstraintEnum {
    CONSTRAINT_TYPE,
    CONSTRAINT_SOURCE,
    CONSTRAINT_TARGET,
    CONSTRAINT_COMNET
  };

public:
  /*! \brief virtual destructor
   *
   */
  virtual ~BConstraint() {}

  ConstraintEnum type() const { return type_; }

  /*! \brief activate this constraint
   *
   *  Some constraints must be satisfied when finishing some queries, so we can
   *  activate/deactivate these constraint checks to save cpu time.
   */
  void activate() { active_ = true; }

  /*! \brief deactivate this constraint
   *
   */
  void deactivate() { active_ = false; }

  /*! \brief weight of this constraint
   *
   *  \attention We haven't used weight to optimize searching now, but it's
   * worth being implemented.
   */
  int weight() const { return weight_; }

  /*! \brief conflict %Variable getter
   *
   *  This is called only when conflict happened.
   *  \param var Which Variable's conflict %Variable you want to get
   *  \return The conflict %Variable
   */
  Variable *get_cause(Variable *var) const {
    return var == var1_ ? var2_ : var1_;
  }

  /*! \brief check if constraint appliable
   *
   *  The %BConstraint is only appliable when the two Variables are both
   * instantiated. \return If constraint appliable
   */
  bool appliable() const;

  /*! \brief check if constraint satisfied
   *
   *  This can only be called when the %BConstraint is appliable. It's overrided
   * by various types of constraint. \return If constraint satisfied
   */
  virtual bool execute() = 0;

protected:
  /*! \brief binary constraint constructor
   *
   *  \param[in] var1 Constraint Variable 1
   *  \param[in] var2 Constraint Variable 2
   *  \param[in] w    Weight of the %BConstraint
   */
  BConstraint(Variable *var1, Variable *var2, int w, ConstraintEnum);

  /*! \brief unary constraint constructor
   *
   *  \param[in] var Constraint Variable
   *  \param[in] w   Weight of the %BConstraint
   */
  BConstraint(Variable *var, int w, ConstraintEnum);

protected:
  /*! \brief constraint %Variable 1
   *
   */
  Variable *var1_;

  /*! \brief constraint %Variable 2
   *
   */
  Variable *var2_;

  /*! \brief weight of the %BConstraint
   *
   */
  int weight_;

private:
  /*! \brief active flag
   *
   */
  bool active_;

  ConstraintEnum type_;
};

/*! \brief class %ConstraintType
 *
 *  This is a unary constraint. It represents the type constraint according the
 * rule GObject. For GNode, it will check type name, connected pin etc. For
 * GArc, it will check type name, source pin name, sink pins name etc.
 */
class ConstraintType : public BConstraint {
public:
  /*! \brief constructor
   *
   */
  ConstraintType(Variable *);

  /*! \brief %ConstraintType checker
   *
   */
  bool execute();
};

/*! \brief class %ConstraintSource
 *
 *  This is a binary constraint. The first Variable is always involved with a
 * rule GNode, and the second is involved with a rule GArc. It will check if the
 * source of the  rule GArc instance is the instance of the rule GNode.
 */
class ConstraintSource : public BConstraint {
public:
  /*! \brief constructor
   *
   */
  ConstraintSource(Variable *, Variable *);

  /*! \brief %ConstraintSource checker
   *
   */
  bool execute();
};

/*! \brief class %ConstraintTarget
 *
 *  This is a binary constraint. The first Variable is always involved with a
 * rule GNode, and the second is involved with a rule GArc. It will check if the
 * target of the  rule GArc instance is the instance of the rule GNode.
 */
class ConstraintTarget : public BConstraint {
public:
  /*! \brief constructor
   *
   */
  ConstraintTarget(Variable *, Variable *);

  /*! \brief %ConstraintTarget checker
   *
   */
  bool execute();
};

/*! \brief class %ConstraintComNet
 *
 *  This is a binary constraint. It represent the common net constraint between
 * the two GNode instance. Both two Variables are involved with a rule GNode and
 * it will check if the two image PKInstances are satisfied to the common net
 * constraint described by the two rule GNode. \warning This constraint is still
 * not tested. By now no rule will produce such constraint.
 */
class ConstraintComNet : public BConstraint {
public:
  /*! \brief constructor
   *
   */
  ConstraintComNet(Variable *, Variable *);

  /*! \brief %ConstraintComNet checker
   *
   */
  bool execute();
};

/*! \brief class %Query
 *
 *  %Query is used to query a domain of a Variable that satisfied specific
 * constraint. That means every %Query involved with a BConstraint. When doing
 * query, we should first instantiate the source Variable, then we find out
 * which instances satisfy the specific constraint with the source instance. For
 * example I have a instance of a Variable(source Variable) involved with a rule
 * GNode and now I want to know what's outgoing GArc can I get, so I use
 * QueryOutgoing involved with ConstraintSource to get the domain of the target
 * Variable involved with a rule GArc.
 */
class Query {
public:
  /*! \brief type of %Query
   *
   *  According to different types of constraint we general have 5 types of
   * query. Maybe you should ask why there's no QUERY_COM_NET, indeed it can be
   * here but I consider the query is waste of time and I just ignore it. Such
   * ignorer is OK because all these 5 types of query are enough to query all
   * domains of Variables. Still I preserve the common net constraint for
   * checking, the match we found will satisfy the rule.
   */
  enum QueryEnum { QUERY_TYPE, QUERY_SRC, QUERY_TAR, QUERY_IN, QUERY_OUT };

public:
  /*! \brief unary %Query constructor
   *
   *  \param[in] var The target Variable we want to query
   *  \param[in] w   The weight of the query
   *  \param[in] bc  The involved constraint
   *  \param[in] qt  The type of the query
   */
  Query(Variable *var, int w, BConstraint *bc, QueryEnum qt);

  /*! \brief binary %Query constructor
   *
   *  \param[in] src The source Variable we use to query
   *  \param[in] tar The target Variable we want to query
   *  \param[in] w   The weight of the query
   *  \param[in] bc  The involved constraint
   *  \param[in] qt  The type of the query
   */
  Query(Variable *src, Variable *tar, int w, BConstraint *bc, QueryEnum qt);

  virtual ~Query() {}

  /*! \brief source %Variable getter
   *
   *  \return The source Variable
   */
  Variable *src_var() const { return source_; }

  /*! \brief target %Variable getter
   *
   *  \return The target Variable
   */
  Variable *tar_var() const { return target_; }

  /*! \brief type of target %Variable getter
   *
   *  This getter is only used by QueryType when calling
   * QueryType::query_gobjects() \return The type of target Variable
   */
  const string &gobj_type() const { return target_->rule_gobj()->type(); }

  /*! \brief type of the %Query
   *
   *  \return The type of the query
   */
  QueryEnum qtype() const { return qtype_; }

  /*! \brief check if query appliable
   *
   *  The query is only appliable when the source Variable is instantiated.
   * QueryType is always appliable because it have no source Variable. \return
   * If the query appliable
   */
  bool appliable() const;

  /*! \brief activate involved constraint
   *
   */
  void activate() {
    if (correspond_)
      correspond_->activate();
  }

  /*! \brief deactivate involved constraint
   *
   *  When we finish a query we always can deactivate the involved constraint
   * for saving cpu time, because the constraint is always satisfied.
   */
  void deactivate() {
    if (correspond_)
      correspond_->deactivate();
  }

  /*! \brief query execution
   *
   *  %Query the domain.
   *  \return The Java like enumerator of the queried domain.
   */
  virtual Variable::DomEnumer execute() = 0;

  /*! \brief size of query result predictor
   *
   *  This routine is used to predict the size of the queried domain. It's
   * useful for constructing search strategy. Always perform small size query is
   * preferred. \return The predict size of the query result
   */
  virtual int size() = 0;

protected:
  /*! \brief weight of the query
   *
   */
  int weight_;

  /*! \brief involved constraint
   *
   */
  BConstraint *correspond_;

  /*! \brief source %Variable
   *
   */
  Variable *source_;

  /*! \brief target %Variable
   *
   */
  Variable *target_;

  /*! \brief query result(queried domain)
   *
   */
  unique_list<GObject *> query_result_;

private:
  /*! \brief type of the query
   *
   */
  QueryEnum qtype_;
};

/*! \brief class %QueryType
 *
 *  This is used to do query according type constraint
 */
class QueryType : public Query {
public:
  /*! \brief constructor
   *
   */
  QueryType(Variable *, BConstraint *);

  /*! \brief calculate average number of incoming arcs
   *
   *  This routine is called by size predictor of QueryIncoming. It calculates
   * the average number of incoming arcs. It only valid for the target Variable
   * involved with a rule GNode.
   */
  int average_in_degree();

  /*! \brief calculate average number of outgoing arcs
   *
   *  This routine is called by size predictor of QueryOutgoing. It calculates
   * the average number of outgoing arcs. It only valid for the target Variable
   * involved with a rule GNode.
   */
  int average_out_degree();

  /*! \brief check if a %GObject is type satisfied
   *
   *  This routine is called by another queries when executing to guarantee all
   *  query results are type satisfied.
   */
  bool type_ok(GObject *gobj) const {
    return query_result_.find(gobj) != query_result_.end();
  }

  /*! \brief [override] %QueryType execution
   *
   */
  Variable::DomEnumer execute();

  /*! \brief [override] size predictor of %QueryType
   *
   */
  int size();

  /*! \brief real %QueryType execution
   *
   *  %QueryType is slightly different from other queries. Because %QueryType
   * not only do the type name check but also some properties check I move all
   * these actions into this routine, it can do it once and get the result
   * everywhere by calling QueryType::execute(). In fact QueryType::execute() is
   * only a result getter.
   */
  void query_gobjects(CellGraph::GObjList *);

private:
  /*! \brief average number of incoming arcs
   *
   */
  int average_in_;

  /*! \brief average number of outgoing arcs
   *
   */
  int average_out_;
};

/*! \brief class %QueryIncoming
 *
 *  This %QueryIncoming is used to query all incoming arcs of a Variable
 * involved with a rule GNode. The source Variable is always involved with a
 * rule GNode and the target one with a rule GArc.
 */
class QueryIncoming : public Query {
public:
  /*! \brief constructor
   *
   */
  QueryIncoming(Variable *, Variable *, BConstraint *);

  /*! \brief [override] %QueryIncoming execution
   *
   */
  Variable::DomEnumer execute();

  /*! \brief [override] size predictor of %QueryIncoming
   *
   */
  int size();
};

/*! \brief class %QueryOutgoing
 *
 *  This %QueryOutgoing is used to query all outgoing arcs of a Variable
 * involved with a rule GNode. The source Variable is always involved with a
 * rule GNode and the target one with a rule GArc.
 */
class QueryOutgoing : public Query {
public:
  /*! \brief constructor
   *
   */
  QueryOutgoing(Variable *, Variable *, BConstraint *);

  /*! \brief [override] %QueryOutgoing execution
   *
   */
  Variable::DomEnumer execute();

  /*! \brief [override] size predictor of %QueryOutgoing
   *
   */
  int size();
};

/*! \brief class %QuerySource
 *
 *  This %QuerySource is used to query the source GNode of a Variable involved
 * with a rule GArc. The source Variable is always involved with a rule GArc and
 * the target one with a rule GNode.
 */
class QuerySource : public Query {
public:
  /*! \brief constructor
   *
   */
  QuerySource(Variable *, Variable *, BConstraint *);

  /*! \brief [override] %QuerySource execution
   *
   */
  Variable::DomEnumer execute();

  /*! \brief [override] size predictor of %QuerySource
   *
   */
  int size();
};

/*! \brief class %QueryTarget
 *
 *  This %QueryTarget is used to query the target GNode of a Variable involved
 * with a rule GArc. The source Variable is always involved with a rule GArc and
 * the target one with a rule GNode.
 */
class QueryTarget : public Query {
public:
  /*! \brief constructor
   *
   */
  QueryTarget(Variable *, Variable *, BConstraint *);

  /*! \brief [override] %QueryTarget execution
   *
   */
  Variable::DomEnumer execute();

  /*! \brief [override] size predictor of %QueryTarget
   *
   */
  int size();
};
} // namespace PACK

#endif