
#include <iostream>

#include "log.h"
#include "report.h"
#include "utils.h"

namespace FDU {
namespace RPT {
using namespace std;
using namespace boost;

///////////////				Element
////////////////////

Element *Element::find_element(const string &id) {
  if (attrs_["id"] == id)
    return this;
  for (Element *child : children())
    if (Element *target = child->find_element(id))
      return target;
  return 0;
}

Element *Element::create_item(const string &id, const string &label,
                              const string &value) {
  Element *item = new Element(this);
  add_child(item)->set_attrs(id, label, value);
  return item;
}

////////////////				report
////////////////////

Section *Report::create_section(const string &id, const string &label) {
  Section *sec = new Section(this);
  add_child(sec)->set_attrs(id, label);
  return sec;
}

void Report::set_item(const string &id, const string &value) {
  Element *target = find_element(id);
  if (target)
    target->set_value(value);
  else
    FDU_LOG(WARN) << "[Report::set_item] Cannot locate target with id: " << id;
}

void Report::set_item(const string &id, int part, int all) {
  if (part > all)
    std::swap(part, all);
  set_item(id, str(format("%d out of %d \t%.2f%%") % part % all %
                   (part * 100. / all)));
}

Table *Report::get_table(const string &id) {
  Element *target = find_element(id);
  if (!target || target->name() != "table") {
    FDU_LOG(WARN) << "[Report::get_table] Failed to get table with id: " << id;
    return 0;
  }
  return static_cast<Table *>(target);
}

////////////////			  section
////////////////////

Table *Section::create_table(const string &id, const string &label) {
  Table *table = new Table(this);
  add_child(table)->set_attrs(id, label);
  return table;
}

////////////////				table
////////////////////

Row *Table::create_row() {
  Row *row = new Row(this);
  add_child(row)->set_id(to_string(++row_count_));
  return row;
}

Column *Table::create_column(const string &id, const string &label) {
  Column *col = new Column(this);
  add_child(col)->set_attrs(id, label);
  return col;
}

////////////////				row
////////////////////

void Row::set_item(const string &id, const string &value) {
  if (children().empty())
    for (Element *col : parent()->children())
      if (col->name() == "column")
        add_child(new Element(this))->set_id(col->attr("id"));
  Element *target = find_element(id);
  if (target)
    target->set_value(value);
  else
    FDU_LOG(WARN) << "[Row::set_item] Failed to locate column with id: " << id;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////		STA Report
//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

static const char *types[] = {"Rise", "Fall"};
static EnumStringMap<TimingType> typemap(types);
std::istream &operator>>(std::istream &s, TimingType &t) {
  t = typemap.readEnum(s);
  return s;
}
std::ostream &operator<<(std::ostream &s, TimingType t) {
  return typemap.writeEnum(s, t);
}

static const char *modes[] = {"R2R", "R2O", "I2R"};
static EnumStringMap<TimingMode> modemap(modes);
std::istream &operator>>(std::istream &s, TimingMode &m) {
  m = modemap.readEnum(s);
  return s;
}
std::ostream &operator<<(std::ostream &s, TimingMode m) {
  return modemap.writeEnum(s, m);
}

//////////////////////////////  STAReport  ///////////////////////////////////

STAReport::STAReport() : Report() {
  set_xslt("sta_report.xsl");
  set_app("STA");
  set_label("unlabeled");
  set_design("empty");
  set_device("empty");
}

STAPart *STAReport::create_part(TimingMode mode) {
  STAPart *part = new STAPart(this);
  add_child(part)->set_attr("mode", to_string(mode));
  return part;
}

void STAPart::set_min_period(double period) {
  set_attr("min_period", to_string("%.2f", period));
  set_attr("max_freq", to_string("%.2f", 1000. / period));
}

STADomain *STAPart::create_domain(const string &net_name) {
  STADomain *dom = new STADomain(this);
  add_child(dom);
  dom->set_attr("clk_net", net_name);
  return dom;
}

//////////////////////////////  STADomain  ///////////////////////////////////

STASection *STADomain::section(TimingType type) {
  if (children().empty())
    for (int t = RISE; t <= FALL; ++t)
      add_child(new STASection(this, (TimingType)t));

  return static_cast<STASection *>(children().begin()[type]);
}

//////////////////////////////  STASection  //////////////////////////////////

STASection::STASection(Element *parent, TimingType type)
    : Element(parent, "section"), path_count_(0) {
  set_attr("type", to_string(type));
}

STAPath *STASection::create_path(const string &src, const string &src_mod,
                                 const string &dst, const string &dst_mod) {
  STAPath *m_path = new STAPath(this);
  add_child(m_path);
  m_path->set_id(to_string(path_count_++));
  m_path->set_attr("src", src);
  m_path->set_attr("src_module", src_mod);
  m_path->set_attr("dst", dst);
  m_path->set_attr("dst_module", dst_mod);
  return m_path;
}

//////////////////////////////  STAPath  //////////////////////////////////

STANode *STAPath::create_node(const string &dir, const string &name,
                              double tarr) {
  STANode *m_node = new STANode(this);
  add_child(m_node);
  m_node->set_attr("dir", dir);
  m_node->set_attr("name", name);
  m_node->set_attr("tarr", to_string("%.2f", tarr));
  return m_node;
}

void STAPath::set_slack(double slack) {
  set_attr("slack", to_string("%.2f", slack));
}

} // namespace RPT
} // namespace FDU

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////		XML
//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#include <rapidxml/rapidxml_print.hpp>
#include <rapidxml/rapidxml_utils.hpp>

namespace {
using namespace FDU::XML;
using namespace FDU::RPT;

void read_attrs(Element *elem, xml_node *node) {
  foreach_attribute(attr, node) elem->set_attr(attr->name(), attr->value());
}

Element *read_item(xml_node *node, Element *parent) {
  Element *item = parent->create_item();
  read_attrs(item, node);
  foreach_child(child, node, ) {
    string node_name = child->name();
    if (node_name == "item")
      read_item(child, item);
    else
      FDU_LOG(WARN) << "read_section: invalid tag name: " << node_name;
  }
  return item;
}

Table *read_table(xml_node *node, Section *parent) {
  Table *table = parent->create_table();
  read_attrs(table, node);
  foreach_child(child, node, ) {
    string node_name = child->name();
    if (node_name == "column")
      read_attrs(table->create_column(), child);
    else
      FDU_LOG(WARN) << "read_table: invalid tag name: " << node_name;
  }
  return table;
}

Section *read_section(xml_node *node, Report *parent) {
  Section *sec = parent->create_section();
  read_attrs(sec, node);
  foreach_child(child, node, ) {
    string node_name = child->name();
    if (node_name == "item")
      read_item(child, sec);
    else if (node_name == "table")
      read_table(child, sec);
    else
      FDU_LOG(WARN) << "read_section: invalid tag name: " << node_name;
  }
  return sec;
}

class ReportWriter : public DomBuilder {
public:
  ReportWriter(const Report *rpt) : DomBuilder() { write_report(rpt); }
  void write_xml(std::ostream &os) const { os << document(); }

private:
  void write_report(const Report *rpt);
  void write_element(const Element *elem, xml_node *parent);
};

void ReportWriter::write_report(const Report *rpt) {
  set_attribute(create_declaration("1.0"), "encoding", "ISO-8859-1");
  create_pi(&document(), "xml-stylesheet",
            str(format("type='text/xsl' href='%1%'") % rpt->xslt()),
            COPY_VALUE);
  write_element(rpt, &document());
}

void ReportWriter::write_element(const Element *elem, xml_node *parent) {
  xml_node *node = create_element(parent, elem->name());
  for (const Element::attrs_type::value_type &attr : elem->attrs())
    if (!attr.second.empty())
      set_attribute(node, attr.first, attr.second);

  for (const Element *child : elem->children())
    write_element(child, node);
}

} // namespace

namespace FDU {
namespace RPT {

typedef rapidxml::file<> xml_file;

void Report::read_template(const string &temp_name) {
  xml_document doc;
  xml_file file_template(temp_name.c_str());
  doc.parse<0>(file_template.data());
  xml_node *root = doc.first_node();
  read_attrs(this, root);

  foreach_child(node, root, ) {
    string node_name = node->name();
    if (node_name == "item")
      read_item(node, this);
    else if (node_name == "section")
      read_section(node, this);
    else
      FDU_LOG(WARN) << "read_template: invalid tag name: " << node_name;
  }
}

void Report::write(const string &file_name) {
  ofstream out(file_name.c_str());
  ReportWriter(this).write_xml(out);
}

} // namespace RPT
} // namespace FDU
