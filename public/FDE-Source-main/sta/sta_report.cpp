#include "sta_report.hpp"
#include "sta_engine.hpp"
#include "sta_utils.hpp"

#include <boost/format.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/range/adaptors.hpp>
#include <map>

// unoptimized, copy from old version

namespace FDU {
namespace STA {

using namespace std;
using namespace boost;
using namespace boost::adaptors;
using namespace STA_RPT_FORMAT;
using namespace ARCH;
using namespace COS;

//////////////////////////////////////////////////////////////////////////
// io streams

static const char *rp_modes[] = {"r2r", "r2o", "i2r"};
static EnumStringMap<STAReport::REPORT_MODE> rp_modes_map(rp_modes);
istream &operator>>(istream &s, STAReport::REPORT_MODE &mode) {
  mode = rp_modes_map.readEnum(s);
  return s;
}
ostream &operator<<(ostream &s, STAReport::REPORT_MODE mode) {
  return rp_modes_map.writeEnum(s, mode);
}

//////////////////////////////////////////////////////////////////////////
// STA Report

void STAReport::export_report(const std::string &fname, int num_paths,
                              REPORT_MODE mode /* = R2R */) {
  ofstream rpt(fname.c_str());
  ASSERT(rpt.is_open(), "can NOT write STA report.");

  report_title(rpt);
  switch (mode) {
  case R2R:
    report_r2r(rpt, num_paths);
    break;
  case R2O:
    report_r2o(rpt, num_paths);
    break;
  case I2R:
    report_i2r(rpt, num_paths);
    break;
  }
  rpt.close();
}

void STAReport::report_title(std::ofstream &rpt) const {
  static boost::format design_fmt("%1% %|30t|%2%");
  static boost::format device_fmt("%1% %|30t|%2%");

  rpt << FULL_DELIM << endl;
  rpt << "Static Timing Analyzer Report" << endl;
  rpt << "Copyright of Fudan University. All rights reserved."
      << "\n\n";
  rpt << design_fmt % "Design file: " % _sta_engine->target_design()->name()
      << endl;
  rpt << device_fmt % "Device: " % FPGADesign::instance()->name() << endl;
  rpt << FULL_DELIM << endl;
}

void STAReport::report_r2r(std::ofstream &rpt, int num_paths) const {
  if (_sta_engine->_dffs.empty()) {
    rpt << "[Warning] NO registers found!" << endl;
    return;
  }

  rpt << "\n\n";
  rpt << FULL_DELIM << endl;
  rpt << "Register to Register Information" << endl;
  rpt << FULL_DELIM << endl;

  double max_period_r = TPara::MINIMUM_FLOAT;
  double max_period_f = TPara::MINIMUM_FLOAT;

  // for each domain
  for (int i = 1; i < _sta_engine->t_domains(); i++) {
    rpt << endl
        << HALF_DELIM << " " << _sta_engine->_clk_nets[i]->name() << " "
        << HALF_DELIM << endl;

    PathSort rise_paths, fall_paths;
    double max_delay_r = TPara::MINIMUM_FLOAT;
    double max_delay_f = TPara::MINIMUM_FLOAT;

    typedef map<Instance *, int> Dffs;
    // find dffs belong to this domain
    // for each po, find the path and record it
    for (auto [dff, t_idx] : _sta_engine->_dffs) {
      if (t_idx != i)
        continue;
      STANode *dff_po = nullptr;
      // find node represent this dff input
      for (STANode *node : _sta_engine->_primary_outputs) {
        if (!node->owner()->is_mpin() && node->owner()->owner() == dff) {
          dff_po = node;
          break;
        }
      }

      ASSERTD(dff_po, "dff does NOT have corresponding primary output.");

      // check po valid at current domain
      if (dff_po->t_arr(i)._rising > 0 && dff_po->t_arr(i)._falling > 0) {

        list<STANode *> rise_path, fall_path;
        // if valid at i domain, find path
        _sta_engine->trace_from_primary_output(dff_po, rise_path, true, i);
        _sta_engine->trace_from_primary_output(dff_po, fall_path, false, i);

        list<STANode *>::iterator it_node_r = rise_path.begin();
        list<STANode *>::iterator it_node_f = fall_path.begin();
        STANode *ptr_po_r = *it_node_r;
        STANode *ptr_po_f = *it_node_f;

        max_delay_r = max(ptr_po_r->t_arr(i)._rising, max_delay_r);
        max_delay_f = max(ptr_po_f->t_arr(i)._falling, max_delay_f);
        max_period_r = max(ptr_po_r->t_arr(i)._rising, max_period_r);
        max_period_f = max(ptr_po_f->t_arr(i)._falling, max_period_f);

        rise_paths.insert(make_pair(ptr_po_r->t_arr(i)._rising, rise_path));
        fall_paths.insert(make_pair(ptr_po_f->t_arr(i)._falling, fall_path));
      }
    } // end of "foreach dff"

    rpt << HALF_DELIM << " Type: Rise " << HALF_DELIM << endl;
    report_paths(rpt, rise_paths, true, max_delay_r, i, num_paths);
    rpt << endl;
    rpt << HALF_DELIM << " Type: Fall " << HALF_DELIM << endl;
    report_paths(rpt, fall_paths, false, max_delay_f, i, num_paths);
    rpt << FULL_DELIM << endl;
  }
  rpt << endl;

  // give the maximum frequency of the chip
  double min_t_period = max(max_period_f, max_period_r);
  if (min_t_period > 0.) {
    rpt << "Minimum Period is " << boost::format("%.2f") % min_t_period << "ns"
        << endl;
    rpt << "Maximum Frequency is "
        << boost::format("%.2f") % (1. / min_t_period * 1e3) << "MHz" << endl;
  } else
    rpt << "[Warning] NO registers found!" << endl;

  rpt << FULL_DELIM << "\n\n\n";
}

void STAReport::report_i2r(std::ofstream &rpt, int num_paths) const {
  if (_sta_engine->_dffs.empty()) {
    rpt << "[Warning] NO registers found!" << endl;
    return;
  }

  rpt << "\n\n";
  rpt << FULL_DELIM << endl;
  rpt << "Input to Register Information" << endl;
  rpt << FULL_DELIM << endl;

  double max_period_r = TPara::MINIMUM_FLOAT;
  double max_period_f = TPara::MINIMUM_FLOAT;

  // 		for(int i = 1; i < _sta_engine->t_domains(); i++)
  // 		{
  int i = 0;
  rpt << endl
      << HALF_DELIM //				<< " " <<
                    //_sta_engine->_clk_nets[i]->name() << " "
      << HALF_DELIM << endl;

  PathSort rise_paths, fall_paths;
  double max_delay_r = TPara::MINIMUM_FLOAT;
  double max_delay_f = TPara::MINIMUM_FLOAT;

  typedef map<Instance *, int> Dffs;
  for (auto [dff, t_idx] : _sta_engine->_dffs) {
    STANode *dff_po = nullptr;
    for (STANode *node : _sta_engine->_primary_outputs) {
      if (!node->owner()->is_mpin() && node->owner()->owner() == dff) {
        dff_po = node;
        break;
      }
    }
    ASSERTD(dff_po, "dff does NOT have corresponding primary output.");

    if (dff_po->t_arr(i)._rising > 0 && dff_po->t_arr(i)._falling > 0) {
      list<STANode *> rise_path, fall_path;
      _sta_engine->trace_from_primary_output(dff_po, rise_path, true, i);
      _sta_engine->trace_from_primary_output(dff_po, fall_path, false, i);

      list<STANode *>::iterator it_node_r = rise_path.begin();
      list<STANode *>::iterator it_node_f = fall_path.begin();
      STANode *ptr_po_r = *it_node_r;
      STANode *ptr_po_f = *it_node_f;

      //					double t_setup_rising  =
      //(*it_node_r)->t_arr(i)._rising  - (*(++it_node_r))->t_arr(i)._rising;
      //					double t_setup_falling =
      //(*it_node_f)->t_arr(i)._falling - (*(++it_node_f))->t_arr(i)._falling;

      max_delay_r =
          max(ptr_po_r->t_arr(i)._rising /*  - t_setup_rising*/, max_delay_r);
      max_delay_f =
          max(ptr_po_f->t_arr(i)._falling /* - t_setup_falling*/, max_delay_f);
      max_period_r = max(ptr_po_r->t_arr(i)._rising, max_period_r);
      max_period_f = max(ptr_po_f->t_arr(i)._falling, max_period_f);

      rise_paths.insert(make_pair(ptr_po_r->t_arr(i)._rising, rise_path));
      fall_paths.insert(make_pair(ptr_po_f->t_arr(i)._falling, fall_path));
    }
  } // end of "foreach dff"

  rpt << HALF_DELIM << " Type: Rise " << HALF_DELIM << endl;
  report_paths(rpt, rise_paths, true, max_delay_r, i, num_paths);
  rpt << endl;
  rpt << HALF_DELIM << " Type: Fall " << HALF_DELIM << endl;
  report_paths(rpt, fall_paths, false, max_delay_f, i, num_paths);
  rpt << FULL_DELIM << endl;
  //		}
  rpt << endl;

  // give the maximum frequency of the chip
  double min_t_period = max(max_period_f, max_period_r);
  if (min_t_period > 0.) {
    rpt << "Minimum Period is " << boost::format("%.2f") % min_t_period << "ns"
        << endl;
    rpt << "Maximum Frequency is "
        << boost::format("%.2f") % (1. / min_t_period * 1e3) << "MHz" << endl;
  } else
    rpt << "[Warning] NO registers found!" << endl;

  rpt << FULL_DELIM << "\n\n\n";
}

void STAReport::report_r2o(std::ofstream &rpt, int num_paths) const {
  if (_sta_engine->_dffs.empty()) {
    rpt << "[Warning] NO registers found!" << endl;
    return;
  }

  rpt << "\n\n";
  rpt << FULL_DELIM << endl;
  rpt << "Register to Output Information" << endl;
  rpt << FULL_DELIM << endl;

  double max_period_r = TPara::MINIMUM_FLOAT;
  double max_period_f = TPara::MINIMUM_FLOAT;

  for (int i = 1; i < _sta_engine->t_domains(); i++) {
    rpt << endl
        << HALF_DELIM << " " << _sta_engine->_clk_nets[i]->name() << " "
        << HALF_DELIM << endl;

    PathSort rise_paths, fall_paths;
    double max_delay_r = TPara::MINIMUM_FLOAT;
    double max_delay_f = TPara::MINIMUM_FLOAT;

    typedef map<Instance *, int> Dffs;
    for (STANode *dff_po : _sta_engine->_primary_outputs) {
      if (!dff_po->owner()->is_mpin())
        continue;
      // ASSERTD(dff_po, Error("dff does NOT have corresponding primary
      // output."));

      if (dff_po->t_arr(i)._rising > 0 && dff_po->t_arr(i)._falling > 0) {
        list<STANode *> rise_path, fall_path;
        _sta_engine->trace_from_primary_output(dff_po, rise_path, true, i);
        _sta_engine->trace_from_primary_output(dff_po, fall_path, false, i);

        list<STANode *>::iterator it_node_r = rise_path.begin();
        list<STANode *>::iterator it_node_f = fall_path.begin();
        STANode *ptr_po_r = *it_node_r;
        STANode *ptr_po_f = *it_node_f;

        //					double t_setup_rising  =
        //(*it_node_r)->t_arr(i)._rising  - (*(++it_node_r))->t_arr(i)._rising;
        //					double t_setup_falling =
        //(*it_node_f)->t_arr(i)._falling - (*(++it_node_f))->t_arr(i)._falling;

        max_delay_r =
            max(ptr_po_r->t_arr(i)._rising /*  - t_setup_rising*/, max_delay_r);
        max_delay_f = max(ptr_po_f->t_arr(i)._falling /* - t_setup_falling*/,
                          max_delay_f);
        max_period_r = max(ptr_po_r->t_arr(i)._rising, max_period_r);
        max_period_f = max(ptr_po_f->t_arr(i)._falling, max_period_f);

        rise_paths.insert(make_pair(ptr_po_r->t_arr(i)._rising, rise_path));
        fall_paths.insert(make_pair(ptr_po_f->t_arr(i)._falling, fall_path));
      }
    } // end of "foreach dff"

    rpt << HALF_DELIM << " Type: Rise " << HALF_DELIM << endl;
    report_paths(rpt, rise_paths, true, max_delay_r, i, num_paths);
    rpt << endl;
    rpt << HALF_DELIM << " Type: Fall " << HALF_DELIM << endl;
    report_paths(rpt, fall_paths, false, max_delay_f, i, num_paths);
    rpt << FULL_DELIM << endl;
  }
  rpt << endl;

  // give the maximum frequency of the chip
  double min_t_period = max(max_period_f, max_period_r);
  if (min_t_period > 0.) {
    rpt << "Minimum Period is " << boost::format("%.2f") % min_t_period << "ns"
        << endl;
    rpt << "Maximum Frequency is "
        << boost::format("%.2f") % (1. / min_t_period * 1e3) << "MHz" << endl;
  } else
    rpt << "[Warning] NO registers found!" << endl;

  rpt << FULL_DELIM << "\n\n\n";
}

void STAReport::report_paths(ofstream &rpt, PathSort &paths, bool rise,
                             double cur_domain_max_delay, int cur_domain,
                             int num_paths) const {
  static boost::format src_des_fmt("%|5t|%s %|20t|%s %|30t|%s");
  static boost::format table_fmt("%|10t|%s %|100t|%s");
  static boost::format pin_fmt("%|10t|%s %|16t|%s %|100t|%.2f");
  static boost::format slack_fmt("%|90t|%s%|100t|%.2f");

  int idx_of_path = 0;
  // reversed for max tarr (or min slack)
  for (PathSort::value_type &path : paths | reversed) {
    rpt << "Data Path " << idx_of_path++ << "\n\n";
    list<STANode *> &nodes = path.second;
    Pin *c = (*nodes.rbegin())->owner();
    string a = (*nodes.rbegin())->owner()->instance()->name();
    // Pin* c = (*nodes.rbegin())->owner();
    Instance *b = (*nodes.rbegin())->owner()->instance();

    rpt << src_des_fmt % "Source: " %
               (*nodes.rbegin())->owner()->instance()->name() %
               (*nodes.rbegin())->owner()->instance()->down_module()->name()
        << endl;

    rpt << src_des_fmt % "Destination: " %
               (*nodes.begin())->owner()->instance()->name() %
               (*nodes.begin())->owner()->instance()->down_module()->name()
        << endl;

    rpt << table_fmt % "Pins: " % "AT" << endl;
    rpt << table_fmt % "------" % "-----" << endl;

    // reversed for starting from primary input
    double tarr;
    for (STANode *node : nodes | reversed) {
      string dir = _sta_engine->_primary_inputs.count(node)    ? "[PI]"
                   : _sta_engine->_primary_outputs.count(node) ? "[PO]"
                                                               : "->";

      string pin_name("");

      switch (node->type()) {
      case STANode::SEQ_SRC:
        pin_name = "Sequential Source";
        break;
      case STANode::SEQ_SINK:
        pin_name = "Sequential Sink";
        break;
      case STANode::CONST_GEN_SRC:
        pin_name = "Constant Generator Source";
        break;
      default:
        pin_name = node->owner()->path_name();
        break;
      }

      tarr = rise ? node->t_arr(cur_domain)._rising
                  : node->t_arr(cur_domain)._falling;
      rpt << pin_fmt % dir % pin_name % tarr << endl;
    }
    rpt << table_fmt % "" % "-----" << endl;
    rpt << slack_fmt % "Slack: " % (cur_domain_max_delay - tarr) << "\n\n";
    if (idx_of_path >= num_paths)
      break;
  }
}

} // namespace STA
} // namespace FDU
