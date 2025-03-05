#include "iclib.hpp"
#include "zfstream.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <rapidxml/rapidxml_utils.hpp> // file
#include <xmlutils.h>

using namespace std;
using namespace FDU::XML;
using namespace boost;

namespace FDU {
namespace STA {

ICLib::pointer ICLib::instance_(new ICLib);

/*
double DummyICPath::lookup(double load, double tran, LUT & lut)
{
        return 0.0;
}
*/

double ICPath::lookup(double load, double tran, LUT &lut) {
  //! find itran position
  int tran_base = -1;
  for (size_t i = 0; i < itran.size() - 1; ++i)
    if (itran[i] <= tran && itran[i + 1] >= tran) {
      tran_base = i;
      break;
    }

  //! find oload position
  int load_base = -1;
  for (size_t i = 0; i < oload.size() - 1; ++i)
    if (oload[i] <= load && oload[i + 1] >= load) {
      load_base = i;
      break;
    }

  //! check itran position
  if (tran_base == -1) {
    if (tran < itran[0]) {
      // FDU_LOG(DEBUG) << "input transition " << tran <<
      // " too small, bound to: " << itran[0] << endl;
      tran_base = 0;
      tran = itran[0];
    } else {
      // FDU_LOG(DEBUG) << "input transition " << tran <<
      // " too big, bound to: " << itran.back() << endl;
      tran_base = itran.size() - 2;
      tran = itran.back();
    }
  }

  //! check oload position
  if (load_base == -1) {
    if (load < oload[0]) {
      // FDU_LOG(DEBUG) << "output load " << load <<
      // " too small, bound to: " << oload[0] << endl;
      load_base = 0;
      load = oload[0];
    } else {
      // FDU_LOG(DEBUG) << "output load " << load <<
      // " too big, bound to: " << oload.back() << endl;
      load_base = oload.size() - 2;
      load = oload.back();
    }
  }

  //! linear delay calculation
  double delay_load_low_bound =
      (lut[load_base + 1][tran_base] - lut[load_base][tran_base]) *
          (load - oload[load_base]) /
          (oload[load_base + 1] - oload[load_base]) +
      lut[load_base][tran_base];

  double delay_load_high_bound =
      (lut[load_base + 1][tran_base + 1] - lut[load_base][tran_base + 1]) *
          (load - oload[load_base]) /
          (oload[load_base + 1] - oload[load_base]) +
      lut[load_base][tran_base + 1];

  double value = (delay_load_high_bound - delay_load_low_bound) *
                     (tran - itran[tran_base]) /
                     (itran[tran_base + 1] - itran[tran_base]) +
                 delay_load_low_bound;

  return value;
}

////////////////////////////////////////////////////////////////////////
// iclib handler

void ICLibHandler::load_iclib(xml_node *node, ICLib *iclib) {
  foreach_child(lib_node, node, "library") load_sublib(lib_node, iclib);
}

void ICLibHandler::load_sublib(xml_node *node, ICLib *iclib) {
  foreach_child(model_node, node, "cell") load_icmodel(model_node, iclib);
}

void ICLibHandler::load_icmodel(xml_node *node, ICLib *iclib) {
  string model_name(get_attribute(node, "name"));
  ASSERT(model_name.size(), "parse iclib failed. cell name not found");
  ICModule *icmodule = new ICModule(model_name);
  foreach_child(path_node, node, "path") load_icpath(path_node, icmodule);
  iclib->add_model(icmodule);
}

void ICLibHandler::load_icpath(xml_node *node, ICModule *icmodule) {
  string path_idx = get_attribute(node, "value");
  ICPath *path = new ICPath(icmodule, lexical_cast<int>(path_idx));

  foreach_child(child_node, node, 0) smart_node_analysis(path, child_node);
  icmodule->add_path(path);
}

void ICLibHandler::load_table(ICPath *path, LUT &lut, const Tokens &tokens) {
  lut.renew(path->x, path->y);

  Tokens::const_iterator it = tokens.begin();
  for (size_t i = 0; i < path->x; ++i)
    for (size_t j = 0; j < path->y; ++j)
      lut[i][j] = lexical_cast<double>(*it++);
}

void ICLibHandler::smart_node_analysis(ICPath *path, xml_node *node) {
  string ename(node->name());
  string node_value(node->value());
  trim(node_value);
  Tokens tokens(node_value, char_separator<char>(" \t\n"));

  if ("cap" == ename) {
    string scin_on = get_attribute(node, "cinon");
    string scin_off = get_attribute(node, "cinoff");
    string scout_off = get_attribute(node, "coutoff");
    if (scin_off.size())
      path->cin_off = lexical_cast<double>(scin_off);
    if (scin_on.size())
      path->cin_on = lexical_cast<double>(scin_on);
    if (scout_off.size())
      path->cout_off = lexical_cast<double>(scout_off);
  } else if ("spice" == ename) {
    path->spice_on = get_attribute(node, "on");
    path->spice_off = get_attribute(node, "off");
  } else if ("delayr" == ename)
    load_table(path, path->delayr, tokens);
  else if ("delayf" == ename)
    load_table(path, path->delayf, tokens);
  else if ("transr" == ename)
    load_table(path, path->transr, tokens);
  else if ("transf" == ename)
    load_table(path, path->transf, tokens);
  else if ("itran" == ename) {
    for (const string &avalue : tokens)
      path->itran.push_back(lexical_cast<double>(avalue));
    path->y = path->itran.size();
  } else if ("oload" == ename) {
    for (const string &avalue : tokens)
      path->oload.push_back(lexical_cast<double>(avalue));
    path->x = path->oload.size();
  } else {
    ASSERT(0, "parse failed. unknown element: " + ename);
  }
}

typedef ::rapidxml::file<> xml_file;

void ICLib::load_lib(const string &file) {
  ifstrm autoifs(file.c_str());
  load(autoifs);
}

void ICLib::load(std::istream &istrm) {
  xml_file file(istrm);
  xml_document doc;
  doc.parse<0>(file.data());
  xml_node *node = doc.first_node();
  ICLibHandler handler;
  if (node->name() == string("device"))
    handler.load_iclib(node, this);
}

ICLib::~ICLib() {
  for (module_map::iterator it = modules_.begin(); it != modules_.end(); ++it)
    if (it->second)
      delete it->second, it->second = 0;
}

} // namespace STA
} // namespace FDU
