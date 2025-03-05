#ifndef IC_LIB_HPP
#define IC_LIB_HPP

#include "matrix.h"
#include "utils.h"
#include "xmlutils.h"

#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <map>
#include <vector>

namespace FDU {
namespace STA {

using namespace FDU;
using namespace FDU::XML;

typedef Matrix<double> LUT;
typedef std::vector<double> Axis;

struct ICPath;
class ICModule;

struct ICPath {
  ICPath(ICModule *owner, size_t id)
      : owner(owner), id(id), x(0), y(0), cin_off(0.), cin_on(0.), cout_off(0.),
        cout_on(0.) {}

  ICModule *owner;
  size_t id;
  size_t x, y;
  std::string spice_on;
  std::string spice_off;
  double cin_off, cin_on, cout_off, cout_on;

  Axis oload;
  Axis itran;
  LUT delayr, delayf;
  LUT transr, transf;

  virtual double lookup_delayr(double load, double tran) {
    return lookup(load, tran, delayr);
  }
  virtual double lookup_delayf(double load, double tran) {
    return lookup(load, tran, delayf);
  }
  virtual double lookup_transr(double load, double tran) {
    return lookup(load, tran, transr);
  }
  virtual double lookup_transf(double load, double tran) {
    return lookup(load, tran, transf);
  }

protected:
  double lookup(double load, double tran, LUT &lut);
};

class DummyICPath : public ICPath {
  DummyICPath(ICModule *owner, size_t id) : ICPath(owner, id) {}

public:
  static ICPath *dummy_icpath() {
    static DummyICPath p(0, 0);
    return &p;
  }

  double lookup_delayr(double load, double tran) { return 0.; }
  double lookup_delayf(double load, double tran) { return 0.; }
  double lookup_transr(double load, double tran) { return 0.; }
  double lookup_transf(double load, double tran) { return 0.; }
};

class ICModule {
public:
  ICModule(const std::string &name) : name_(name) {}
  ~ICModule() {
    for (ICPath *path : paths_)
      if (path)
        delete path, path = 0;
  }

  ICPath *add_path(ICPath *path) {
    paths_.push_back(path);
    return path;
  }
  ICPath *get_path(int i) {
    ASSERT(i >= 0 && i < paths_.size(),
           "illegal access of path ID: " + boost::lexical_cast<string>(i));
    return paths_[i];
  }

  std::string name() const { return name_; }

private:
  std::string name_;
  std::vector<ICPath *> paths_;
};

class ICLib {
public:
  typedef std::map<std::string, ICModule *> module_map;
  typedef boost::scoped_ptr<ICLib> pointer;

  ~ICLib();

  void load_lib(const std::string &fname);

  ICModule *add_model(ICModule *model) {
    modules_.insert(std::make_pair(model->name(), model));
    return model;
  }

  ICModule *get_model(const std::string &name) {
    module_map::iterator it = modules_.find(name);
    return it != modules_.end() ? it->second : nullptr;
  }

  static ICLib *instance() { return instance_.get(); }

private:
  void load(std::istream &istrm);

  module_map modules_;
  static pointer instance_;
};

class ICLibHandler {
public:
  virtual void load_iclib(xml_node *node, ICLib *iclib);
  virtual void load_sublib(xml_node *node, ICLib *iclib);

protected:
  virtual void load_icmodel(xml_node *node, ICLib *iclib);
  virtual void load_icpath(xml_node *node, ICModule *icmodel);

  typedef boost::tokenizer<boost::char_separator<char>> Tokens;
  void load_table(ICPath *path, LUT &lut, const Tokens &tokens);
  void smart_node_analysis(ICPath *path, xml_node *node);
};

} // namespace STA
} // namespace FDU

#endif
