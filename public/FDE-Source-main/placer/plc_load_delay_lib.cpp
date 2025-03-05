#include "plc_load_delay_lib.h"
#include "io/fileio.hpp"
#include "plc_device.h"
#include "xmlutils.h"
#include "zfstream.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <rapidxml/rapidxml_utils.hpp>

namespace FDU {
namespace Place {

using namespace std;
using namespace boost;
using namespace FDU;
using namespace COS::IO;
using namespace DEVICE;

void DelayTableHandler::load_design(xml_node *node) {
  DeviceInfo::resize_delay_table(DEVICE::NUM_DELAY_TABLE_TYPE);
  foreach_child(child, node, 0) load_delay(child);
}

void DelayTableHandler::load_delay(xml_node *node) {
  static char_separator<char> seq(" \t\n");
  this->_delay_table_type =
      lexical_cast<DelayTableType>(get_attribute(node, "name"));
  this->_delay_table_scale = lexical_cast<Point>(get_attribute(node, "scale"));
  string content = node->value();
  trim(content);
  load_table(DeviceInfo::delay_table(_delay_table_type), _delay_table_scale,
             Tokens(content, seq));
}

void DelayTableHandler::load_table(DeviceInfo::DelayTable &table,
                                   const Point &scale, const Tokens &tokens) {
  table.renew(scale.x, scale.y);
  Tokens::const_iterator it = tokens.begin();
  for (size_t row = 0; row < scale.x; ++row)
    for (size_t col = 0; col < scale.y; ++col)
      table.at(row, col) = lexical_cast<double>(*it++);
}

class DelayLoader : public Loader { // XML design file
public:
  DelayLoader() : Loader("delay") {}
  void load(std::istream &istrm);
};

void DelayLoader::load(std::istream &istrm) {
  typedef rapidxml::file<> xml_file;
  xml_file file(istrm);
  xml_document doc;
  doc.parse<0>(file.data());
  DelayTableHandler nlh;
  nlh.load_design(doc.first_node());
}

static DelayLoader DelayLoader;
} // namespace Place
} // namespace FDU