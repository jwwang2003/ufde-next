#include <boost/algorithm/string.hpp>
#include <map>

#include "io/fileio.hpp"
#include "netlist.hpp"
#include "zfstream.h"

using namespace std;
using namespace boost;

namespace {
using namespace COS;

template <typename T> class Repository {
  using map_type = map<string, vector<T *>>;
  map_type _map;

public:
  T *get(const string &type) const {
    auto it = _map.find(type);
    if (it == _map.end())
      return 0;
    const vector<T *> &vec = it->second;
    if (vec.empty())
      return 0;
    return vec.back();
  }

  void reg(const string &type, T *t) { _map[type].push_back(t); }

  void unreg(const string &type, T *t) {
    auto it = _map.find(type);
    if (it == _map.end())
      return;
    vector<T *> &vec = it->second;
    auto vit = std::find(vec.begin(), vec.end(), t);
    if (vit != vec.end())
      vec.erase(vit);
  }
};

Repository<IO::Loader> &repo_loader() {
  static Repository<IO::Loader> repo;
  return repo;
}

Repository<IO::Writer> &repo_writer() {
  static Repository<IO::Writer> repo;
  return repo;
}

Property<bool> USED;
Property<vector<Library *>> USED_LIB;
Property<vector<Module *>> USED_MODULE;

inline void mark_module_used(Module *module) { // sort libs and modules
  if (module->property_value(USED))
    return;
  module->set_property(USED, true);
  Library *lib = module->library();
  if (!lib->is_external()) {
    for (Instance *inst : module->instances())
      mark_module_used(inst->down_module());
  }
  if (!lib->property_value(USED)) {
    lib->set_property(USED, true);
    lib->design()->property_ptr(USED_LIB)->push_back(lib);
    lib->set_property(USED_MODULE, vector<Module *>());
  }
  lib->property_ptr(USED_MODULE)->push_back(module);
}

} // namespace

namespace COS {
namespace IO {

Loader::Loader(const string &type) : _type(type), _design(0) {
  to_lower(_type);
  repo_loader().reg(_type, this);
}
Loader::~Loader() { repo_loader().unreg(_type, this); }

void Loader::load(const std::string &file) {
  ifstrm ifs(file.c_str());
  load(ifs);
}

Writer::Writer(const string &type) : _type(type), _design(0) {
  to_lower(_type);
  repo_writer().reg(_type, this);
}
Writer::~Writer() { repo_writer().unreg(_type, this); }

void Writer::write(const std::string &file, bool encrypt) const {
  if (encrypt) {
    zofstream zofs(file.c_str());
    write(zofs);
  } else {
    std::ofstream ofs(file.c_str());
    write(ofs);
  }
}

void Writer::mark_used() const {
  USED.clear();
  USED_MODULE.clear();
  design()->set_property(USED_LIB, vector<Library *>());
  mark_module_used(design()->top_module());
}

const vector<Library *> &Writer::used_lib(const Design *design) {
  return *design->property_ptr(USED_LIB);
}
const vector<Module *> &Writer::used_module(const Library *lib) {
  return *lib->property_ptr(USED_MODULE);
}

Loader *get_loader(string type, Design *design) {
  Loader *loader = repo_loader().get(type);
  if (!loader)
    return 0;
  loader->set_design(design);
  return loader;
}

Writer *get_writer(string type, Design *design) {
  Writer *writer = repo_writer().get(type);
  if (!writer)
    return 0;
  writer->set_design(design);
  return writer;
}

} // namespace IO
} // namespace COS
