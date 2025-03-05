/*! \file   CellGraph.h
 *  \brief  Header of netlist graph description
 *  \author S.C. Leung
 *  \date   2009.05 - 2009.06
 *
 */

#ifndef _CELL_GRAPH_H
#define _CELL_GRAPH_H

#include <fstream>

#include "PKUtils.h"
#include "Rule.h"

namespace PACK {

using COS::Object;
using COS::Pin;

/*! \brief Type information about GObject
 *
 * This class include all common type information about
 * both GNode and GArc
 *
 * \sa GNodeType, GArcType
 */
class GObjType : boost::noncopyable {
  friend class GObject;

public:
  /*! \brief GObject type enum
   *
   * Class GNode with type GNODE and class GArc with type GARC,
   * generally type NOT_SET will not be used.
   *
   */
  enum TypeEnum { NOT_SET, GNODE, GARC };

  /*! \brief virtual destructor
   *
   *  Because it will use GObjType pointer to delete GNodeType and GArcType
   *  objects the destructor should be virtual.
   */
  virtual ~GObjType(){};

protected:
  /*! \brief constructor
   *
   *  This constructor can only be accessed by GNodeType and GArcType.
   *  \param[in] type The type name
   *  \param[in] kind The GObject type
   */
  GObjType(const string &type, TypeEnum kind)
      : type_(type), kind_(kind), rule_lock_(false), layer_lock_(false) {}

  /*! \brief type name of GObject
   *
   *  I use type name of an Instance as GNode type name,
   *  for GArc, I concatenate source Instance type name and
   *  sink Instance type name with "_ARC_" as its type name.
   *  The type name greatly helps type lookup.
   */
  string type_;

  /*! \brief type enum of GObject
   *
   */
  TypeEnum kind_;

  /*! \brief locker within rule
   *
   *  If the attribute "lock" of an Instance within RuleCell is false
   *  it can be matched repeatedly, otherwise it can only be matched once.
   *  I just ignore the GObject if rule_lock_ is true to implement this
   *  behavior. Rule_lock_ will be updated after each match found.
   */
  bool rule_lock_;

  /*! \brief locker within layer
   *
   *  Each rule within layer is exclusive. That means when a GObject matches one
   * rule it will not match another rule within the same layer. Layer_lock_ will
   * be updated after each layer finished matching.
   */
  bool layer_lock_;
};

/*! \brief Type Information of GNode
 *
 *  All Information about GNode. Some may just have meaning for rule GNode.
 *  \sa GObjType
 */
class GNodeType : public GObjType {
  friend class GNode;

public:
  /*! \brief constructor
   *
   *  \param[in] type Type name of GNode
   *  \param[in] o    Corresponding PKInstance
   */
  GNodeType(const string &type, PKInstance *o)
      : GObjType(type, GObjType::GNODE), ori_inst_(o) {}

private:
  /*! \brief corresponding PKInstance
   *
   *  Every GNode correspond a PKInstance.
   */
  PKInstance *ori_inst_;

  /*! \brief must connected pins' name
   *
   *  This member as part of type information, writes down all pins
   *  that should be connected. It's only significant for rule GNode.
   */
  std::vector<cstr_ptr> connect_pins_;
};

/*! \brief Type Information of GArc
 *
 *  All information about GArc. Some may just have meaning for rule GArc.
 *  \sa GObjType
 */
class GArcType : public GObjType {
  friend class GArc;

public:
  /*! \brief sink number compare behavior enum
   *
   *  This enum type is used as net type constraint when finding match.
   *  When the net in RuleCell haven't connected to port the sink number
   *  of image net should be equal to the one of rule net. When the net
   *  connects to port and the "connected" attribute is true, the sink
   *  number of image net should be more than the one of rule net. Otherwise
   *  it should be NOT_LESS.
   */
  enum SinkNumType { EQUAL, MORE, NOT_LESS };

public:
  /*! \brief constructor
   *
   *  \param[in] src_type Type name of the source Instance
   *  \param[in] tar_type Type name of the target Instance
   *  \param[in] o        Corresponding PKNet
   */
  GArcType(const string &src_type, const string &tar_type, PKNet *o);

private:
  /*! \brief corresponding PKNet
   *
   *  Every GArc correspond a PKNet, but a PKNet may correspond many GArcs.
   */
  PKNet *ori_net_;

  /*! \brief sink number constraint type
   *
   *  This member is significant only for rule GArc
   */
  SinkNumType sink_num_type_;

  /*! \brief source pin name of GArc
   *
   *  This member as part of type information specific the source pin name,
   *  It's important for finding match correctly. It may be updated dynamically
   *  when netlist is changed.
   */
  cstr_ptr src_name_;

  /*! \brief sink pins's names of GArc
   *
   *  This member as part of type information specific the sink pins' names,
   *  It's important for finding match correctly. It may be updated dynamically
   *  when netlist is changed.
   */
  std::list<cstr_ptr> sink_names_;
};

// forward declaration
class GNode;
class GArc;
class CellGraph;

/*! \brief class GObject
 *
 *  I abstract graph nodes and arcs as GObject for convenience.
 *  \sa GNode, GArc
 */
class GObject : boost::noncopyable {
public:
  /*! \brief virtual destructor
   *
   *  Because it will use GObject pointer to delete GNode and GArc objects
   *  the destructor should be virtual.
   */
  virtual ~GObject() {}

  /*! \brief corresponding CellGraph
   *
   *  \return The owner graph of GObject
   */
  CellGraph *get_context() const { return context_; }

  /*! \brief type name of GObject
   *
   *  \return The type name of GObject
   */
  const string &type() const { return obj_type_->type_; }

  /*! \brief real type of GObject
   *
   *  \return The real type of GObject, GNODE or GARC
   */
  GObjType::TypeEnum kind() const { return obj_type_->kind_; }

  /*! \brief if it is locked within a layer
   *
   *  \return if it is locked within a layer
   */
  bool layer_lock() const { return obj_type_->layer_lock_; }

  /*! \brief if it is locked within a rule
   *
   *  \return if it is locked within a rule
   */
  bool rule_lock() const { return obj_type_->rule_lock_; }

  /*! \brief if it needs to lock within a rule
   *
   *  \return if it needs to lock within a rule
   */
  bool rule_to_lock() const { return rule_to_lock_; }

  /*! \brief set need to lock within a rule
   *
   *  \param[in] b Whether need to lock within a rule
   */
  void set_rule_to_lock(bool b) { rule_to_lock_ = b; }

  /*! \brief set the locker within a layer
   *
   *  It will be called after every match found
   */
  void set_layer_lock() { obj_type_->layer_lock_ = true; }

  /*! \brief clear the locker within a layer
   *
   *  It will be called after finishing a layer match
   */
  void clear_layer_lock() { obj_type_->layer_lock_ = false; }

  /*! \brief set the locker within a rule
   *
   *  It will be called if it need to lock within a rule after a match found
   */
  void set_rule_lock() { obj_type_->rule_lock_ = true; }

  /*! \brief clear the locker within a rule
   *
   *  It will be called after finishing a layer match
   */
  void clear_rule_lock() { obj_type_->rule_lock_ = false; }

  /*! \brief suicide
   *
   *  \warning It should be a pure virtual function, but for a bug of
   *  ptr_container in boost 1.36, I implement it as empty function.
   *  Maybe you should update to boost 1.38 or later to fix it.
   */
  virtual void release(){};

  /*! \brief echo dot file for drawing graph
   *
   *  It's a debug interface.
   *  \warning It should be a pure virtual function, but for a bug of
   *  ptr_container in boost 1.36, I implement it as empty function.
   *  Maybe you should update to boost 1.38 or later to fix it.
   */
  virtual void echo_dotfile(std::ofstream &) const {};

protected:
  /*! \brief constructor
   *
   *  This constructor would only be called by GNode and GArc constructor.
   *  \param[in] context Owner graph
   *  \param[in] type    GObjType object
   */
  GObject(CellGraph *context, GObjType *type)
      : context_(context), rule_to_lock_(true), obj_type_(type) {}

  /*! \brief GObjType getter
   *
   *  It would only be called by GNode and GArc constructor.
   *  \return GObjType pointer
   */
  GObjType *type_ptr() { return obj_type_.get(); }

private:
  /*! \brief corresponding cell graph
   *
   */
  CellGraph *context_;

  /*! \brief flag specific if need to lock within rule
   *
   *  \attention Maybe you can move this member to GObjType for clarity,
   *  but don't be confused with the rule locker.
   */
  bool rule_to_lock_;

  /*! \brief type information of GObject
   *
   *  I use scoped_ptr for auto memory management, release myself from memory
   * hell. You can see my boost memory management tricks in various place.
   */
  boost::scoped_ptr<GObjType> obj_type_;
};

/*! \brief class %GNode
 *
 *  Node type of cell graph.
 */
class GNode : public GObject {
public:
  /*! \brief constructor
   *
   *  \param[in] context Corresponding CellGraph
   *  \param[in] type    Type name of %GNode
   *  \param[in] o       Corresponding PKInstance
   */
  GNode(CellGraph *context, const string &type, PKInstance *o);

  /*! \brief number of incoming arcs
   *
   *  Incoming arc means the %GNode is the target node of the GArc
   *  \return Number of incoming arcs
   */
  size_t num_incoming_arcs() const { return incoming_arcs_.size(); }

  /*! \brief number of outgoing arcs
   *
   *  Outgoing arc means the %GNode is the source node of the GArc
   *  \return Number of outgoing arcs
   */
  size_t num_outgoing_arcs() const { return outgoing_arcs_.size(); }

  /*! \brief arc iteration typedef
   *
   *  Boost iterator_range typedef of GArc, used to iterate incoming and
   * outgoing arcs.
   */
  typedef boost::iterator_range<std::list<GArc *>::const_iterator>
      const_arcs_type;

  /*! \brief incoming arcs iterator
   *
   *  Iterator all incoming arcs.
   *  \return iterator_range of GArc.
   */
  const_arcs_type incoming_arcs() const {
    return const_arcs_type(incoming_arcs_.begin(), incoming_arcs_.end());
  }

  /*! \brief outgoing arcs iterator
   *
   *  Iterator all outgoing arcs.
   *  \return iterator_range of GArc.
   */
  const_arcs_type outgoing_arcs() const {
    return const_arcs_type(outgoing_arcs_.begin(), outgoing_arcs_.end());
  }

  /*! \brief add outgoing arc
   *
   *  This function is called by GArc constructor. We should always bind high
   *  coupling jobs to reduce possibility of error.
   *  \param[in] arc Outgoing arc to be added.
   */
  void add_outgoing(GArc *arc) { outgoing_arcs_.push_back(arc); }

  /*! \brief add incoming arc
   *
   *  This function is called by GArc constructor.
   *  \param[in] arc Incoming arc to be added.
   */
  void add_incoming(GArc *arc) { incoming_arcs_.push_back(arc); }

  /*! \brief remove outgoing arc
   *
   *  This function is called by GArc::release().
   *  \param[in] arc Outgoing arc to be removed.
   */
  void remove_outgoing(GArc *arc) { outgoing_arcs_.remove(arc); }

  /*! \brief remove incoming arc
   *
   *  This function is called by GArc::release().
   *  \param[in] arc Incoming arc to be removed.
   */
  void remove_incoming(GArc *arc) { incoming_arcs_.remove(arc); }

  /*! \brief suicide
   *
   *  Release corresponding resources like incoming & outgoing arcs, type
   * collections within CellGraph etc.
   */
  void release();

  /*! \brief corresponding PKInstance
   *
   *  \return Corresponding PKInstance
   */
  PKInstance *correspond() const { return node_type_->ori_inst_; }

  /*! \brief typedef of pin name iterator
   */
  typedef boost::iterator_range<std::vector<cstr_ptr>::const_iterator>
      const_pstr_type;

  /*! \brief connected pins' name iterator
   *
   *  It's only significant for rule %GNode
   *  \return Pin name iterator
   */
  const_pstr_type connect_pin() const {
    return const_pstr_type(node_type_->connect_pins_.begin(),
                           node_type_->connect_pins_.end());
  }

  /*! \brief pin names vector
   *
   *  It's only used by rule %GNode
   */
  typedef std::vector<cstr_ptr> PinNames;

  /*! \brief pair of PinNames
   *
   *  I use this structure to note down the common net pins of two Instances.
   *  PinNamesPair.first note one Instance's common net pins, PinNamePair.second
   *  note the other Instance's common net pins. It's only used by rule %GNode.
   */
  typedef std::pair<PinNames, PinNames> PinNamesPair;

  /*! \brief PinNamesPair vector
   *
   *  I use this structure to note down all the common net pins between two
   * Instances. It's only used by rule %GNode.
   */
  typedef std::vector<PinNamesPair> ComNetPairs;

  /*! \brief type CommonNetMap
   *
   *  I use this map to find out any common net information corresponding to
   * this %GNode. It's only used by rule %GNode.
   */
  struct CommonNet {
    GNode *target;
    ComNetPairs net_pairs;
    explicit CommonNet(GNode *n) : target(n) {}
  };

  typedef std::vector<CommonNet> CommonNetMap;

  /*! \brief iterator_range of CommonNetMap
   *
   */
  typedef boost::iterator_range<CommonNetMap::const_iterator> const_comnet_type;

  /*! \brief common net information of specific %GNode
   *
   *  It's used for checking common net constraint.
   *  \param[in] n Specific %GNode
   *  \return Common net information
   */
  ComNetPairs &get_common_net(GNode *n);

  /*! \brief getter of common net information about this %GNode
   *
   *  It's used for creating common net constraint when building constraints.
   *  \return Iterator_range of CommonNetMap
   */
  const_comnet_type common_nets() const {
    return const_comnet_type(common_net_map_.begin(), common_net_map_.end());
  }

  /*! \brief create a common net information
   *
   *  It's only be used by rule %GNode.
   *  \param[in] n Common net information for specific %GNode n
   *  \return Common net information holder
   */
  PinNamesPair *create_pname_pair(GNode *n);

  /*! \brief add connected pin
   *
   *  It's only used by rule %GNode.
   *  \param[in] cpstr Required connected pin name.
   */
  void add_connect_pin(cstr_ptr cpstr) {
    node_type_->connect_pins_.push_back(cpstr);
  }

  /*! \brief echo dot file about %GNode for drawing graph
   *
   *  It's a debug interface.
   *  \param[in] o Output stream
   */
  void echo_dotfile(std::ofstream &o) const;

private:
  /*! \brief type information of %GNode
   *
   *  It's a counterpart of GObject::obj_type_, I use it to prevent boring
   * static_cast
   */
  GNodeType *node_type_;

  /*! \brief outgoing arcs
   *
   *  I use list to store arcs for the convenience of updating outgoing arcs.
   */
  std::list<GArc *> outgoing_arcs_;

  /*! \brief incoming arcs
   *
   *  I use list to store arcs for the convenience of updating incoming arcs.
   */
  std::list<GArc *> incoming_arcs_;

  /*! \brief common net information about this %GNode
   *
   *  It's only significant for rule %GNode.
   *  \warning The correctness of this structure's creation is not tested! By
   * now no rule requires common net constraint.
   */
  CommonNetMap common_net_map_;
};

/*! \brief class GArc
 *
 *  Arc type for cell graph
 */
class GArc : public GObject {
public:
  /*! \brief pin name type enum
   *
   *  Specific the type of the pin name
   */
  enum PinType { SRC_NAME, SINK_NAME };

public:
  /*! \brief constructor
   *
   *  \param[in] context Owner graph
   *  \param[in] source_ Source node of the %GArc
   *  \param[in] target_ Target node of the %GArc
   *  \param[in] o       Corresponding PKNet
   */
  GArc(CellGraph *context, GNode *source_, GNode *target_, PKNet *o);

  /*! \brief source node getter
   *
   *  \return Source GNode of the %GArc
   */
  GNode *source() const { return source_; }

  /*! \brief target node getter
   *
   *  \return Target GNode of the %GArc
   */
  GNode *target() const { return target_; }

  /*! \brief suicide
   *
   *  Release corresponding resource like incoming/outgoing arc of its
   * target/source GNode, corresponding PKNet, type collector of CellGraph etc.
   */
  void release();

  /*! \brief pin name iterator_range
   *
   */
  typedef boost::iterator_range<std::list<cstr_ptr>::const_iterator>
      const_pstr_type;

  /*! \brief corresponding PKNet
   *
   *  \return Corresponding PKNet
   */
  PKNet *correspond() const { return arc_type_->ori_net_; }

  /*! \brief constraint type of sink number
   *
   *  \return Constraint type of sink number
   */
  GArcType::SinkNumType sink_num_type() const {
    return arc_type_->sink_num_type_;
  }

  /*! \brief source pin name
   *
   *  \return Source pin name
   */
  const string &src_pin_name() const { return *arc_type_->src_name_; }

  /*! \brief number of pure sinks of corresponding net
   *
   *  \return Number of pure sinks of corresponding net
   */
  int num_pure_sink_of_net() const {
    return arc_type_->ori_net_->num_pure_sinks();
  }

  /*! \brief number of pure sinks on this %GArc
   *
   *  It's used to decide if this %GArc should release
   *  when the corresponding net unhook a pin.
   *  \return Number of pure sinks on this %GArc
   */
  size_t num_sinks() const { return arc_type_->sink_names_.size(); }

  /*! \brief iterator_range of sink pin names
   *
   *  \return Iterator_range of sink pin names
   */
  const_pstr_type sink_pin_names() const {
    return const_pstr_type(arc_type_->sink_names_.begin(),
                           arc_type_->sink_names_.end());
  }

  /*! \brief constraint type of sink number setter
   *
   *  \param[in] t Constraint type of sink number
   */
  void set_sink_num_type(GArcType::SinkNumType t) {
    arc_type_->sink_num_type_ = t;
  }

  /*! \brief remove a sink on this %GArc
   *
   *  It's used to update CellGraph dynamically. This should only be called by
   * CellGraph in theory, maybe you should use friend to guarantee this
   * protocol. \param[in] cpstr Sink pin name that removed
   */
  void remove_sink_pname(cstr_ptr cpstr) {
    arc_type_->sink_names_.remove(cpstr);
  }

  /*! \brief add a sink on this %GArc
   *
   *  It's used to update CellGraph dynamically. This should only be called by
   * CellGraph in theory, maybe you should use friend to guarantee this
   * protocol. \param[in] cpstr Sink pin name that added
   */
  void add_sink_pname(cstr_ptr cpstr) {
    arc_type_->sink_names_.push_back(cpstr);
  }

  /*! \brief add source/sink pin name
   *
   *  \param[in] pt Type of pin name, SRC_NAME or SINK_NAME
   *  \param[in] cpstr Pin name
   */
  void set_pin_name(PinType pt, const string &cpstr);

  /*! \brief echo dot file of %GArc for drawing graph
   *
   *  It's a debug interface.
   *  \param[in] o Output stream
   */
  void echo_dotfile(std::ofstream &o) const;

private:
  /*! \brief type information of %GArc
   *
   *  It's a counterpart of GObject::obj_type_, I use it to prevent boring
   * static_cast
   */
  GArcType *arc_type_;

  /*! \brief source node of %GArc
   *
   */
  GNode *source_;

  /*! \brief target node of %GArc
   *
   */
  GNode *target_;
};

/*! \brief class %CellGraph
 *
 *  The graph style description of a netlist.
 */
class CellGraph {
  friend class PKCell;
  friend class GNode;
  friend class GArc;

public:
  /*! \brief typedef of GObject list
   *
   */
  typedef std::list<GObject *> GObjList;

  /*! \brief typedef of GObject type map
   *
   *  I use this for fast %GObject lookup by %GObject type name.
   */
  typedef std::map<string, GObjList> TypeGObjMap;

public:
  /*! \brief constructor
   *
   *  Each PKCell can be convert to a CellGraph.
   *  \param[in] cell Corresponding PKCell
   */
  CellGraph(PKCell *cell) : context_(cell), output_graph_idx_(0) {}

  /*! \brief build graph routine
   *
   *  This routine include building PKCell and RuleCell graph. When
   *  building graph with specific PKCell, each PKInstance in PKCell will be
   *  converted to a GNode, each PKNet in PKCell will be converted to a set of
   * GArc.
   *
   */
  void build_graph();

  /*! \brief classify GObject
   *
   *  This routing will fill the CellGraph::type_map_ which is the type lookup
   * map for GObject. \deprecated This routine is substituted by type insertion
   * in CellGraph::create_gnode() and CellGraph::create_garc() routines.
   */
  void fill_type_map();

  /*! \brief type find routine
   *
   *  \param[in] gt Type name of GObject
   *  \return List of GObjects found, nullptr returned when find nothing
   */
  GObjList *find_gobjs(const string &gt);

  /*! \brief graph drawing routine
   *
   *  It's a debug interface. The output format is *.png.
   */
  void draw();

  /*! \brief GObject container
   *
   *  I use ptr_set because of the demand for fast GObject insertion and
   * deletion.
   */
  // zhouxg: use PtrList to eliminate the randomness
  typedef PtrList<GObject> GObjPtrList;

  /*! \brief iterator_range of GObject
   *
   */
  typedef GObjPtrList::const_range_type const_gobjs_type;

  /*! \brief iterator_range of GObject
   *
   */
  typedef GObjPtrList::range_type gobjs_type;

  /*! \brief GObjects getter
   *
   */
  const_gobjs_type gobjects() const { return gobjects_.range(); }

  /*! \brief GObjects getter
   *
   */
  gobjs_type gobjects() { return gobjects_.range(); }

  /*! \brief clear layer lock of all GObjects
   *
   *  It will be called after finishing a layer match.
   */
  void clear_layer_lock();

  /*! \brief clear rule lock of all GObjects
   *
   *  It will be called after finishing a layer match.
   */
  void clear_rule_lock();

private:
  /*! \brief RuleCell graph build routine
   *
   *  This routine will fill the information specific for rule GNode and rule
   * GArc.
   */
  void build_rule_cell();

  /*! \brief clear the graph
   *
   *  This routine will delete all GObjects and clear the CellGraph::type_map_.
   *  I firstly want to allow change a PKCell within the CellGraph's life time,
   *  but it's proved not a good idea. So maybe each graph will be build once,
   *  I still keep calling clear() in build_graph() for safety.
   */
  void clear();

  /*! \brief create a GNode
   *
   *  This routine will initialize most work that required for creating a GNode,
   *  like add it to CellGraph::gobjects_, insert it to type map etc. The
   * parameters is the same as the constructor of GNode.
   */
  inline GNode *create_gnode(CellGraph *, const string &, PKInstance *);

  /*! \brief create a GArc
   *
   *  This routine will initialize most work that required for creating a GArc,
   *  like add it to CellGraph::gobjects_, insert it to type map etc. The
   * parameters is the same as the constructor of GArc.
   */
  inline GArc *create_garc(CellGraph *, GNode *, GNode *, PKNet *);

  /*! \brief slot correspond to cell change signal
   *
   *  This will be called after an Instance created. It will create a
   * corresponding GNode. \param[in] inst Instance that created
   */
  void after_inst_created(PKInstance *inst);

  /*! \brief slot correspond to cell change signal
   *
   *  This will be called before pin unhooked. It will update the corresponding
   * GArc. \param[in] net Net that unhooked \param[in] pin Pin that unhooked
   */
  void before_pin_unhooked(PKNet *net, PKPin *pin);

  /*! \brief slot correspond to cell change signal
   *
   *  This will be called before pin hookuped. It will update the corresponding
   * GArc. \param[in] net Net that hookuped \param[in] pin Pin that hookuped
   */
  void before_pin_hookuped(PKNet *net, PKPin *pin);

  /*! \brief slot correspond to cell change signal
   *
   *  This will be called before pin rehook. It will update the corresponding
   * GArc. \param[in] old_net Net that unhooked \param[in] new_net Net that
   * hookuped \param[in] pin     Pin that rehook
   */
  void before_pin_rehook(PKNet *old_net, PKNet *new_net, PKPin *pin);

  /*! \brief slot correspond to cell change signal
   *
   *  This will be called before net removed. It will delete the corresponding
   * GArc. \param[in] net Net that removed
   */
  void before_net_remove(PKNet *net);

  /*! \brief slot correspond to cell change signal
   *
   *  This will be called before instance removed. It will delete the
   * corresponding GNode. \param[in] inst Instance that removed
   */
  void before_inst_remove(PKInstance *inst);

private:
  /*! \brief corresponding PKCell
   *
   */
  PKCell *context_;

  /*! \brief GObjects container
   *
   */
  GObjPtrList gobjects_;

  /*! \brief type map of GObjects
   *
   */
  TypeGObjMap type_map_;

  /*! \brief output graph index
   *
   *  Because graphviz cannot overwrite an exist file, to achieve output graph
   *  of the same cell continuously I append the index to the output file name.
   */
  int output_graph_idx_;
};

inline GNode *CellGraph::create_gnode(CellGraph *context, const string &type,
                                      PKInstance *o) {
  GNode *gnode = new GNode(context, type, o);
  gobjects_.push_back(gnode);
  type_map_[gnode->type()].push_back(gnode);
  return gnode;
}

inline GArc *CellGraph::create_garc(CellGraph *context, GNode *source_,
                                    GNode *target_, PKNet *o) {
  GArc *garc = new GArc(context, source_, target_, o);
  gobjects_.push_back(garc);
  type_map_[garc->type()].push_back(garc);
  return garc;
}
} // namespace PACK

#endif