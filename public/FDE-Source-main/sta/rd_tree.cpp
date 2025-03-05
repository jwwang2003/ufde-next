#include "rd_tree.hpp"
#include "tengine/tengine.hpp"
#include <boost/regex.hpp>

using namespace std;
using namespace boost;
using namespace FDU::RRG;

namespace FDU {
namespace STA {

static Property<Point> &POS = create_property<Point>(INSTANCE, "position");

RDFactory::pointer RDFactory::_instance(new RDFactory);

void RDNode::recursive_print_cap() {
  if ((type() != SWITCH && type() != DUMMY) || status() == ON) {
    FDU_LOG(DEBUG) << name() << " cap out: " << cap_out();
    FDU_LOG(DEBUG) << name() << " cap in : " << cap_in();
  }
  for (RDNode *child : children())
    child->recursive_print_cap();
}

void RDNode::recursive_print_timing() {
  if ((type() != SWITCH && type() != DUMMY) || status() == ON) {
    FDU_LOG(DEBUG) << name() << " arr_r: " << t_arr().rise
                   << "\tarr_f: " << t_arr().fall
                   << "\ttran_r: " << t_tran().rise
                   << "\ttran_f: " << t_tran().fall;
  }

  for (RDNode *child : children())
    child->recursive_print_timing();
}

void RDNode::calc_cap() {
  // 1. calc extern out cap
  double c_ext = 0.;
  for (RDNode *child : children()) {
    child->calc_cap();
    c_ext += child->cap_in();
  }

  // 2. calc self cap
  if (type() == SWITCH) {
    // get corresponding icpath
    ICPath *path = ic_path();

    if (status() == OFF) {
      // actually when the switch is off,
      // it's not important to calc it's out cap
      // for the out cap will never be used
      // SO: cout_off is not needed

      // set_cap_out(path->cout_off + c_ext);
      set_cap_in(path->cin_off);
    } else if (status() == ON) {
      switch (rrg_sw()->type()) {
      // BUF: cin & cout are separated
      case RRGSwitch::BUF:
        set_cap_out(path->cout_on + c_ext);
        set_cap_in(path->cin_on);
        break;

      // DUMMY: actually not exists
      case RRGSwitch::DUMMY:
        set_cap_out(c_ext);
        set_cap_in(c_ext);
        break;

      // PT: cin = cout + cin_intrinsic
      case RRGSwitch::PT:
        set_cap_out(c_ext + path->cout_on);
        set_cap_in(cap_out() + path->cin_on);
        break;

      // unknown
      default:
        ASSERT(0, "unknown rrg switch type");
        break;
      }
    } else {
      ASSERTD(0, "unknown status");
    }
  } // switch

  else if (type() == LINE) {
    // same as PT: cin = cout + cin_intrinsic
    // however, is it really accurate to do
    // such an estimation
    ICPath *path = ICLib::instance()->get_model("LINE")->get_path(0);
    set_cap_out(c_ext + path->cout_on);
    set_cap_in(path->cin_on + cap_out());
  }

  else /* if (type() == INPUT || type() == OUTPUT || type() == DUMMY) */
  {
    set_cap_out(c_ext);
    if (type() == DUMMY)
      set_cap_in(c_ext);
  }

#ifdef EXPORT_CAP
  FDU_LOG(DEBUG) << name() << " cap out: " << cap_out();
  FDU_LOG(DEBUG) << name() << " cap in : " << cap_in();
#endif
}

void RDNode::calc_delay() {
  RDData arr(RDPARA::DEF_ARR_RISE, RDPARA::DEF_ARR_FALL);
  RDData tran(RDPARA::DEF_TRAN_RISE, RDPARA::DEF_TRAN_FALL);

  // get arr & tran value after signal pass through this node
  // PRE: this node's arr & tran must be ready
  update_arr_and_tran(arr, tran);

#ifdef EXPORT_TIME
  FDU_LOG(DEBUG) << name() << " arr_r: " << t_arr().rise
                 << "\tarr_f: " << t_arr().fall << "\ttran_r: " << t_tran().rise
                 << "\ttran_f: " << t_tran().fall;
#endif

  for (RDNode *child : children()) {
    // set child's arr & tran value
    child->set_t_arr(arr);
    child->set_t_tran(tran);
    child->calc_delay();
  }
}

// Pre: the tarr & ttran of cur node is valid
void RDNode::update_arr_and_tran(RDData &arr, RDData &tran) {
  ICPath *path = 0;
  if ((type() == SWITCH || type() == DUMMY) && status() == ON)
    path = ic_path();
  else if (type() == LINE)
    path = ICLib::instance()->get_model("LINE")->get_path(0);

  if (path) {
    arr.rise = t_arr().rise + path->lookup_delayr(cap_out(), t_tran().rise);
    arr.fall = t_arr().fall + path->lookup_delayf(cap_out(), t_tran().fall);
    tran.rise = path->lookup_transr(cap_out(), t_tran().rise);
    tran.fall = path->lookup_transf(cap_out(), t_tran().fall);
  }
}

void RDTree::calc_all_nodes_load() {
  set_onode_cap_in();
  root()->calc_cap();
}

void RDTree::calc_all_nodes_delay() { root()->calc_delay(); }

void RDTree::set_delay_data() {
  for (const node_pin_pair &onode_pin : snk_nodes_) {
    Pin *pin = onode_pin.second;
    RDNode *node = onode_pin.first;

    Property<double> &ARR_R = create_property<double>(PIN, "arrR");
    Property<double> &ARR_F = create_property<double>(PIN, "arrF");
    pin->set_property(ARR_R, node->t_arr().rise);
    pin->set_property(ARR_F, node->t_arr().fall);

    Property<TData> &DELAY = create_temp_property<TData>(PIN, "delay");
    pin->set_property(DELAY, TData(node->t_arr().rise, node->t_arr().fall));
  }
}

void RDTree::set_onode_cap_in() {
  for (const node_pin_pair &onode_pin : snk_nodes_) {
    TPort *tport = static_cast<TPort *>(onode_pin.second->port());
    onode_pin.first->set_cap_in(tport->capacitance());
    FDU_LOG(VERBOSE) << "\tset sink pin cap: " << tport->capacitance() << endl;
  }
}

void RDTree::build_src_node(const RRGraph &rrg) {
  Pin *src = *owner_->source_pins().begin();
  Point pos = src->instance()->property_value(POS);
  RRGNode *src_node = rrg.find_logic_pin_node(src->instance(), *src, pos);
  ASSERT(src_node, "cannot find rrg node for source pin " + src->path_name());

  FDU_LOG(VERBOSE) << "source: " << src_node->full_name() << endl;
  src_seg_.first = src_node;
  src_seg_.second = src;
}

void RDTree::build_snk_nodes(const RRGraph &rrg) {
  for (Pin *pin : owner_->sink_pins()) {
    Point pos = pin->instance()->property_value(POS);
    RRGNode *src_node = rrg.find_logic_pin_node(pin->instance(), *pin, pos);
    ASSERT(src_node, "cannot find rrg node for source pin " + pin->path_name());
    snk_segs_.insert(make_pair(src_node, pin));
    FDU_LOG(VERBOSE) << "sink: " << src_node->full_name() << endl;
  }
}

void RDTree::build_pips(const RRGraph &rrg) {
  seg_conn_map conn_repo;
  seg_visit_map nodes_repo;
  for (PIP *pip : owner_->routing_pips()) {
    RRGNode *from = rrg.find_grm_net_node(pip->from(), pip->position());
    RRGNode *to = rrg.find_grm_net_node(pip->to(), pip->position());
    ASSERTS(from && to);
    conn_repo[from].push_back(to);
    nodes_repo.insert(make_pair(from, false));
    nodes_repo.insert(make_pair(to, false));
  }

  dfs_visit_seg(src_seg_.first, nodes_repo, conn_repo); // topo sort pips

  for (const Pip &pip : pips()) {
    FDU_DEBUG("from: " + pip.get<0>()->full_name());
    FDU_DEBUG("to  : " + pip.get<2>()->full_name());
  }
}

void RDTree::dfs_visit_seg(RRGNode *node, RDTree::seg_visit_map &nodes_repo,
                           RDTree::seg_conn_map &conn_repo) {
  segs out_nodes = conn_repo[node];
  for (RRGNode *out_node : out_nodes) {
    RRGSwitch *sw = node->find_switch(out_node);
    ASSERTSD(sw);
    pips_.push_back(boost::make_tuple(node, sw, out_node));
    if (!nodes_repo[out_node])
      dfs_visit_seg(out_node, nodes_repo, conn_repo);
  }
  nodes_repo[node] = true;
}

void RDTree::dump_spice_netlist(const std::string &name) {
  string file_name = name + ".sp";
  ofstream sp(file_name.c_str());
  ASSERT(sp, "cannot open file: " << file_name);

  RDNode *input = root();
  sp << ".SUBCKT " << name << " IN ";
  for (node_pin_map::iterator it = sink_nodes().begin();
       it != sink_nodes().end(); it++)
    sp << " " << it->first->index();
  sp << endl;

  // sp << "*density = " << density << endl;

  int i = 0;
  dump_spice_node(input, sp, i);
  sp << ".ENDS" << endl;
  sp.close();
}

void RDTree::dump_spice_node(RDNode *driver, ofstream &sp, int &i) {
  if (driver->type() == RDNode::OUTPUT) {
    // output
    RDNode *parent = driver->parent();
    sp << "*** OUTPUT *** " << driver->name() << endl;
    sp << "R" << i++ << " " << driver->index() << " " << parent->index()
       << " 0 " << endl;
  } else if (driver->type() == RDNode::SWITCH ||
             driver->type() == RDNode::DUMMY) {
    // switch
    ICPath *path = driver->ic_path();
    string st = path->spice_off;

    regex in("IN");
    regex out("OUT");

    RDNode *pre = driver->parent();

    if (driver->status() == RDNode::OFF) {
      st = regex_replace(st, in, lexical_cast<string>(pre->index()));
      st = regex_replace(st, out, lexical_cast<string>(driver->index()));
      sp << "*** Off in Switch *** " << driver->name() << endl;
      sp << "X" << i++ << " " << st << " " << driver->icmod() << endl;
    } else if (driver->status() == RDNode::ON) {
      st = path->spice_on;
      st = regex_replace(st, in, lexical_cast<string>(pre->index()));
      st = regex_replace(st, out, lexical_cast<string>(driver->index()));
      sp << "*** On Switch *** " << driver->name() << endl;
      sp << "X" << i++ << " " << st << " " << driver->icmod() << endl;
    } else {
      st = regex_replace(st, in, lexical_cast<string>(pre->index()));
      st = regex_replace(st, out, lexical_cast<string>(driver->index()));
    }
  } else if (driver->type() == RDNode::LINE) {
    // line
    ICPath *path = driver->ic_path();
    string st = path->spice_on;

    regex in("IN");
    regex out("OUT");

    RDNode *pre = driver->parent();
    st = regex_replace(st, in, lexical_cast<string>(pre->index()));
    st = regex_replace(st, out, lexical_cast<string>(driver->index()));

    sp << "*** Line *** " << driver->name() << endl;
    sp << "X" << i++ << " " << st << " LINE" << endl;
  } else if (driver->type() == RDNode::INPUT) {
    sp << "*** INPUT *** " << driver->name() << endl;
    sp << "R" << i++ << " IN " << driver->index() << " 0 " << endl;
  }

  for (RDNode *child : driver->children()) { // handle child
    dump_spice_node(child, sp, i);
  }
}

void RDTree::dump_dot_file(const std::string &name) {
  string file_name = name + ".dot";
  ofstream dot(file_name.c_str());
  ASSERT(dot, "cannot open file: " << file_name);

  dot << "digraph\tG\t{" << endl;
  RDNode *input = root();
  dump_dot_node(input, dot);
  dot << "}" << endl;

  dot.close();
}

void RDTree::dump_dot_node(RDNode *driver, ofstream &dot) {
  for (RDNode *child : driver->children()) { // handle child
    dot << "\"" << driver->name() << "\"\t->\t\"" << child->name() << "\";"
        << endl;
    dump_dot_node(child, dot);
  }
}

/*
 *
 *
 *
 *void IGraph::dumpSpNl(const string& name, double density)
 *{
 *    string file = name + ".sp";
 *    ofstream sp;
 *    sp.open(file.c_str());
 *
 *    INode* input = this->getInput();
 *
 *    sp << ".SUBCKT " << name << " IN ";
 *
 *    for(map<INode*, Pin*>::iterator it = this->getOutputs().begin(); it !=
 *this->getOutputs().end(); it++) sp << " " << it->first->getIdx();
 *
 *    sp << endl;
 *
 *    sp << "*density = " << density << endl;
 *
 *    int i = 0;
 *    //set<string> sws;
 *    dumpSpSt(input, sp, i[>, sws<]);
 *
 *    sp << ".ENDS" << endl;
 *    sp.close();
 *}
 *
 *
 *
 *void IGraph::dumpSpSt(INode* driver, ofstream& sp, int& i[>, set<string>&
 *sws<])
 *{
 *    if (driver->getType() == INodeType::OUTPUT) {
 *        INode* pre = static_cast<INode*>(*(driver->preNodes().begin()));
 *        sp << "*** OUTPUT *** " << driver->name() << endl;
 *        sp << "R" << i++ << " "<< driver->getIdx() << " " << pre->getIdx() <<
 *" 0 " << endl;
 *    }
 *    else if (driver->getType() == INodeType::SW) {
 *
 *        ArchPath* ap = driver->getSwitch()->owner();
 *        ICModel* model = ICLib::instance()->getModel(ap->owner()->name());
 *        ICPath* path = model->getPath(ap->type());
 *
 *        //sws.insert(ap->owner()->name());
 *
 *        string st = path->_spice_off;
 *
 *        regex in("IN");
 *        regex out("OUT");
 *
 *        INode* pre = static_cast<INode*>(*(driver->preNodes().begin()));
 *
 *        if (driver->getStat() == INodeStat::OFF) {
 *            st = regex_replace(st, in, lexical_cast<string>(pre->getIdx()));
 *            st = regex_replace(st, out,
 *lexical_cast<string>(driver->getIdx())); sp << "*** Off in Switch *** " <<
 *driver->name() << endl; sp << "X" << i++ << " " << st << " " <<
 *ap->owner()->name() << endl;
 *        }
 *        else if (driver->getStat() == INodeStat::ON) {
 *            st = path->_spice_on;
 *            st = regex_replace(st, in, lexical_cast<string>(pre->getIdx()));
 *            st = regex_replace(st, out,
 *lexical_cast<string>(driver->getIdx())); sp << "*** On Switch *** " <<
 *driver->name() << endl; sp << "X" << i++ << " " << st << " " <<
 *ap->owner()->name() << endl;
 *        }
 *        else {
 *            st = regex_replace(st, in, lexical_cast<string>(pre->getIdx()));
 *            st = regex_replace(st, out,
 *lexical_cast<string>(driver->getIdx()));
 *        }
 *
 *        //sp << "X" << i++ << " " << st << endl;
 *    }
 *    else if (driver->getType() == INodeType::LN) {
 *        ICModel* model = ICLib::instance()->getModel("LINE");
 *        ICPath* path = model->getPath(0);
 *
 *        string st = path->_spice_on;
 *
 *        regex in("IN");
 *        regex out("OUT");
 *
 *        INode* pre = static_cast<INode*>(*(driver->preNodes().begin()));
 *        st = regex_replace(st, in, lexical_cast<string>(pre->getIdx()));
 *        st = regex_replace(st, out, lexical_cast<string>(driver->getIdx()));
 *
 *        sp << "*** Line *** " << driver->name() << endl;
 *        sp << "X" << i++ << " " << st << " LINE" << endl;
 *    }
 *    else if (driver->getType() == INodeType::INPUT) {
 *        sp << "*** INPUT *** " << driver->name() << endl;
 *        sp << "R" << i++ << " IN " << driver->getIdx() << " 0 " << endl;
 *    }
 *
 *    for (Node* adj: driver->adjNodes()) {
 *        INode* iadj = static_cast<INode*>(adj);
 *        dumpSpSt(iadj, sp, i[>, sws<]);
 *    }
 *}
 *
 *
 *void IGraph::dumpDot(const string& name)
 *{
 *    string file = name + ".dot";
 *    ofstream dot;
 *    dot.open(file.c_str());
 *
 *    dot << "digraph\tG\t{" << endl;
 *
 *    INode* input = this->getInput();
 *
 *    dumpDotSt(input, dot);
 *
 *    dot << "}" << endl;
 *
 *    dot.close();
 *}
 *
 *void IGraph::dumpDotSt(INode* driver, ofstream& dot)
 *{
 *    for (Node* adj: driver->adjNodes()) {
 *        INode* iadj = static_cast<INode*>(adj);
 *        dot << "\"" << driver->name() << "\"\t->\t\"" << iadj->name() << "\";"
 *<< endl; dumpDotSt(iadj, dot);
 *    }
 *}
 *
 *
 *
 */

RDNode *RDFactory::make_node(const std::string &name, RDNode::RDNODE_TYPE type,
                             RDNode *parent, RRGSwitch *sw /* = 0 */) {
  return new RDNode(name, type, parent, sw);
}

RDTreePtr RDFactory::make_tree(COSRTNet *net) {
  return RDTreePtr(new RDTree(net));
}

} // namespace STA
} // namespace FDU
