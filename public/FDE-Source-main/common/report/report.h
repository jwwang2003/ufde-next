#ifndef _NODE_TYPE_H
#define _NODE_TYPE_H

#include "netlist/ptrcontainer.hpp"
#include "xmlutils.h" // to_string
#include <iosfwd>
#include <map>
#include <string>

namespace FDU {
namespace RPT {
using COS::PtrVector;
using std::string;

class Element {
public:
  typedef std::map<string, string> attrs_type;
  typedef boost::iterator_range<attrs_type::const_iterator> attrs_range;
  typedef PtrVector<Element>::const_range_type children_range;

  Element(Element *parent, const string &name = "item")
      : parent_(parent), name_(name) {}

  Element *parent() const { return parent_; }
  const string &name() const { return name_; }
  const string &attr(const string &id) { return attrs_[id]; }
  attrs_range attrs() const { return attrs_range(attrs_); }
  children_range children() const { return children_.range(); }

  void set_attr(const string &name, const string &value) {
    attrs_[name] = value;
  }
  void set_id(const string &value) { set_attr("id", value); }
  void set_label(const string &value) { set_attr("label", value); }
  void set_value(const string &value) { set_attr("value", value); }
  void set_attrs(const string &id, const string &label,
                 const string &value = "");

  Element *create_item(const string &id = "", const string &label = "",
                       const string &value = "");

protected:
  Element *add_child(Element *child) { return children_.add(child); }
  Element *find_element(const string &id);

private:
  Element *parent_;
  string name_;
  attrs_type attrs_;
  PtrVector<Element> children_;
};
inline void Element::set_attrs(const string &id, const string &label,
                               const string &value) {
  if (!id.empty())
    set_id(id);
  if (!label.empty())
    set_label(label);
  if (!value.empty())
    set_value(value);
}

class Section;
class Table;
class Column;
class Row;

class Report : public Element {
public:
  Report() : Element(0, "report") { set_xslt("fdu_report.xsl"); }

  void set_app(const string &value) { set_attr("app", value); }
  void set_design(const string &value) { set_attr("design", value); }
  void set_xslt(const string &xslt_name) { xslt_ = xslt_name; }

  void set_item(const string &id, const string &value);
  template <typename T> void set_item(const string &id, const T &value) {
    set_item(id, FDU::to_string(value));
  }
  void set_item(const string &id, int part, int all);

  const string &xslt() const { return xslt_; }
  Table *get_table(const string &id);

  Section *create_section(const string &id = "", const string &label = "");

  void read_template(const string &file_name);
  void write(const string &file_name);

private:
  string xslt_;
};

class Section : public Element {
public:
  Section(Element *parent) : Element(parent, "section") {}
  Table *create_table(const string &id = "", const string &label = "");
};

class Table : public Element {
public:
  Table(Element *parent) : Element(parent, "table"), row_count_(0) {}
  Column *create_column(const string &id = "", const string &label = "");
  Row *create_row();

private:
  int row_count_;
};

class Column : public Element {
public:
  Column(Element *parent) : Element(parent, "column") {}
};

class Row : public Element {
public:
  Row(Element *parent) : Element(parent, "row") {}
  void set_item(const string &id, const string &value);
  template <typename T> void set_item(const string &id, const T &value) {
    set_item(id, FDU::to_string(value));
  }
};

////////////////////////////////////////////////
///////////      STA report      ///////////////
////////////////////////////////////////////////

enum TimingType { RISE, FALL };
enum TimingMode { R2R, R2O, I2R };

std::istream &operator>>(std::istream &s, TimingType &t);
std::ostream &operator<<(std::ostream &s, TimingType t);
std::istream &operator>>(std::istream &s, TimingMode &m);
std::ostream &operator<<(std::ostream &s, TimingMode m);

class STAPart;
class STADomain;
class STASection;
class STAPath;
class STANode;

class STAReport : public Report {
public:
  STAReport();
  void set_device(const string &device) { set_attr("device", device); }
  STAPart *create_part(TimingMode mode);
};

class STAPart : public Element {
public:
  STAPart(Element *parent) : Element(parent, "part") {}
  void set_min_period(double);
  STADomain *create_domain(const string &net_name);
};

class STADomain : public Element {
public:
  STADomain(Element *parent) : Element(parent, "domain") {}
  STASection *section(TimingType type);
};

class STASection : public Element {
public:
  STASection(Element *parent, TimingType type);
  STAPath *create_path(const string &src, const string &src_mod,
                       const string &dst, const string &dst_mod);

private:
  int path_count_;
};

class STAPath : public Element {
public:
  STAPath(Element *parent) : Element(parent, "path") {}
  STANode *create_node(const string &dir, const string &name, double tarr);
  void set_slack(double);
};

class STANode : public Element {
public:
  STANode(Element *parent) : Element(parent, "node") {}
};

} // namespace RPT
} // namespace FDU

#endif
