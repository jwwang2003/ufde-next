#include "loadedif.hpp"
#include "io/fileio.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <memory>
#include <regex>
#include <stdint.h> // int64_t

#define ASSERTLN(a, s)                                                         \
  ASSERT(a, (format("Parsing Error, line %d: %s") % _line % (s)))

using namespace std;
using namespace boost;
using namespace COS;

namespace { // rename map
using COS::EDIF::Rename;
class RnKey {
  const Object *_owner;
  string _name;

public:
  RnKey(const Object *owner, const string &type, const string &name)
      : _owner(owner), _name(type + name) {}
  bool operator<(const RnKey &rhs) const {
    if (_owner < rhs._owner)
      return true;
    if (_owner > rhs._owner)
      return false;
    return _name < rhs._name;
  }
};

using RnPtr = std::shared_ptr<Rename>;
using RnMap = std::map<RnKey, RnPtr>;
RnMap rename_map;

void add_rename(const Object *owner, const string &type, const RnPtr &rnp) {
  rename_map[RnKey(owner, type, rnp->new_name)] = rnp;
}
Rename *get_rename(const Object *owner, const string &type,
                   const string &name) {
  RnMap::iterator p = rename_map.find(RnKey(owner, type, name));
  return p == rename_map.end() ? 0 : p->second.get();
}
void add_port_renames(const Module *mod) {
  for (const Port *p : mod->ports())
    if (p->is_vector())
      add_rename(mod, "port", RnPtr(new Rename(p->name(), p->width())));
}
} // namespace

namespace COS {
namespace EDIF {

Rename::Rename(const string &nname, const string &oname)
    : new_name(nname), is_bus(false), msb(0), lsb(0) {
  static regex bus_ex("(.+)[\\[\\(<](\\d+):(\\d+)[\\]\\)>]");
  smatch what;
  if (regex_match(oname, what, bus_ex)) {
    is_bus = true;
    old_name = what[1];
    msb = lexical_cast<int>(what[2]);
    lsb = lexical_cast<int>(what[3]);
  } else
    old_name = oname;
}
Rename::Rename(const string &name, int size)
    : old_name(name), new_name(name), is_bus(true), msb(size - 1), lsb(0) {}

const string &Rename::name() const {
  static regex complex_ex("\\$.*\\$.*");
  // use the new name if the old name is too complex
  return regex_match(old_name, complex_ex) ? new_name : old_name;
}

Parser::Parser(Design *design, istream &is)
    : _funcs({{"edif", &Parser::parse_continue},
                          {"rename", &Parser::parse_rename},
                          {"external", &Parser::parse_library},
                          {"library", &Parser::parse_library},
                          {"cell", &Parser::parse_cell},
                          {"celltype", &Parser::parse_continue},
                          {"view", &Parser::parse_continue},
                          {"design", &Parser::parse_design},
                          {"cellref", &Parser::parse_cellref},
                          {"viewref", &Parser::parse_continue},
                          {"libraryref", &Parser::parse_libraryref},
                          {"interface", &Parser::parse_continue},
                          {"contents", &Parser::parse_continue},
                          {"port", &Parser::parse_port},
                          {"direction", &Parser::parse_direction},
                          {"instance", &Parser::parse_instance},
                          {"net", &Parser::parse_net},
                          {"joined", &Parser::parse_continue},
                          {"portref", &Parser::parse_portref},
                          {"instanceref", &Parser::parse_instanceref},
                          {"property", &Parser::parse_property},
                          {"string", &Parser::parse_continue},
                          {"integer", &Parser::parse_continue},
                          {"number", &Parser::parse_continue},
                          {"e", &Parser::parse_e},
                          {"array", &Parser::parse_array},
                          {"member", &Parser::parse_member}}), cur_design(design), cur_lib(0), cur_module(0), cur_net(0),
      _source(is), _line(1) {
  auto &pRename = create_temp_property<string>(DESIGN, "cell_lib_rename");
  auto pvRename = design->property_value<string>(pRename);
  static regex rename_ex("(.+)=(.+)");
  smatch what;
  if (regex_match(pvRename, what, rename_ex)) {
    _cell_lib_new = what[1];
    _cell_lib_old = what[2];
  }
}

inline COS::Object *Parser::cur_obj() {
  ASSERTLN(!obj_stack.empty(), "no current COS object");
  return obj_stack.back();
}

int Parser::getc() {
  int ch = _source.get();
  if (ch == '\n')
    _line++;
  return ch;
}

void Parser::ungetc(char ch) {
  if (ch == '\n')
    _line--;
  _source.putback(ch);
}

Token Parser::get_token() {
  int ch;
  string buf;
  do {
    ch = getc();
  } while (isspace(ch));
  switch (ch) {
  case '(':
    return T_LPAR;
  case ')':
    return T_RPAR;
  case EOF:
    return T_EOF;
  case '"':
    ch = getc();
    while (ch != '"') {
      ASSERTLN(ch != EOF, "EOF in string");
      buf.push_back(ch);
      ch = getc();
    }
    return Token(T_QSTR, buf);
  default:
    while (ch != '(' && ch != ')' && !isspace(ch) && ch != EOF) {
      buf.push_back(ch);
      ch = getc();
    }
    ungetc(ch);
    if (buf[0] == '-' || isdigit(buf[0])) {
      int64_t val;
      try {
        val = boost::lexical_cast<int64_t>(buf);
      } catch (...) {
        ASSERTLN(0, "integer error");
      }
      return Token(T_INT, val);
    }
    return Token(T_NAME, buf);
  }
}

Token Parser::parse_ignore() {
  int count = 1;
  Token tok;
  do {
    tok = get_token();
    if (tok.type() == T_LPAR)
      count++;
    else if (tok.type() == T_RPAR)
      count--;
  } while (count && tok.type() != T_EOF);
  return Token(T_NULL);
}

Token Parser::parse() {
  Token tok = get_token();
  if (tok.type() != T_LPAR)
    return tok;

  tok = get_token();
  ASSERTLN(tok.type() == T_NAME, "keyword required");
  string key = tok.value<string>();
  to_lower(key);
  if (_funcs.find(key) == _funcs.end())
    return parse_ignore();
  else
    return (this->*_funcs[key])();
}

Token Parser::parse_continue() {
  Token tok(T_NULL);
  for (;;) {
    Token last_tok = tok;
    tok = parse();
    if (tok.type() == T_EOF)
      return tok;
    if (tok.type() == T_RPAR)
      return last_tok;
  }
}

Token Parser::parse_rename() {
  Token tok = parse();
  ASSERTLN(tok.type() == T_NAME, "rename: name required");
  string name = tok.value<string>();
  tok = parse();
  ASSERTLN(tok.type() == T_QSTR, "rename: string required");
  string oname = tok.value<string>();
  parse_ignore();
  return Token(T_RENAME, RnPtr(new Rename(name, oname)));
}

string Parser::get_name(const Token &tok, const Object *owner,
                        const string &type) const {
  string name;
  if (tok.type() == T_NAME)
    name = tok.value<string>();
  else if (tok.type() == T_RENAME) {
    RnPtr rnp = tok.value<RnPtr>();
    ASSERTLN(!rnp->is_bus || rnp->bus_size() == 1,
             type + ": bus name not allowed");
    name = rnp->name();
    add_rename(owner, type, rnp);
  } else
    ASSERTLN(0, type + " name required");
  return name;
}

string Parser::get_refname(const Token &tok, const Object *owner,
                           const string &type) const {
  string name;
  ASSERTLN(tok.type() == T_NAME, type + " name required");
  name = tok.value<string>();
  Rename *rn = get_rename(owner, type, name);
  if (!rn)
    return name;
  if (!rn->is_bus)
    return rn->name();
  ASSERTLN(rn->bus_size() == 1, type + "Ref: bus name not allowed");
  return rn->name() + "[" + to_string(rn->lsb) + "]";
}

Token Parser::parse_library() {
  string name = get_name(parse(), cur_design, "library");
  if (name == _cell_lib_old)
    name = _cell_lib_new;
  push_obj(cur_lib = cur_design->find_or_create_library(name));
  Token res = parse_continue();
  pop_obj();
  return res;
}

Token Parser::parse_cell() {
  ASSERTLN(cur_lib, "cell: no current library");
  string name = get_name(parse(), cur_lib, "cell");
  if (const Module *mod = cur_lib->modules().find(name)) { // cell already exist
    add_port_renames(mod);
    return parse_ignore();
  }
  Token tok = parse();
  ASSERTLN(tok.type() == T_NAME, "cell type required");
  string type = tok.value<string>();
  push_obj(cur_module = cur_lib->create_module(name, type));
  Token res = parse_continue();
  pop_obj();
  return res;
}

Token Parser::parse_design() {
  string name = get_name(parse(), 0, "design");
  cur_design->rename(name);
  push_obj(cur_design);
  Token tok = parse();
  ASSERTLN(tok.type() == T_CREF, "design: cellRef required");
  Module *cell = tok.value<Module *>();
  cur_design->set_top_module(cell);
  Token res = parse_continue();
  pop_obj();
  return res;
}

Token Parser::parse_cellref() {
  string name = get_refname(parse(), cur_lib, "cell");
  Library *lib = cur_lib;
  Token tok = parse();
  if (tok.type() != T_RPAR) {
    ASSERTLN(tok.type() == T_LREF, "cellRef: libraryRef required");
    lib = tok.value<Library *>();
    parse_ignore();
  }
  Module *cell = lib->modules().find(name);
  ASSERTLN(cell, "cell not found");
  return Token(T_CREF, cell);
}

Token Parser::parse_libraryref() {
  string name = get_refname(parse(), cur_design, "library");
  if (name == _cell_lib_old)
    name = _cell_lib_new;
  Library *lib = cur_design->libs().find(name);
  ASSERTLN(lib, "library not found");
  parse_ignore();
  return Token(T_LREF, lib);
}

Token Parser::parse_port() {
  ASSERTLN(cur_module, "port: no current cell");
  Token tok = parse();
  if (tok.type() == T_ARRAY) {
    RnPtr rnp = tok.value<RnPtr>();
    Token tdir = parse();
    ASSERTLN(tdir.type() == T_DIR, "port direction required");
    DirType dir = tdir.value<DirType>();
    push_obj(cur_module->create_port(rnp->name(), rnp->msb, rnp->lsb, dir));
  } else {
    string name = get_name(tok, cur_module, "port");
    Token tdir = parse();
    ASSERTLN(tdir.type() == T_DIR, "port direction required");
    DirType dir = tdir.value<DirType>();
    push_obj(cur_module->create_port(name, dir));
  }
  Token res = parse_continue();
  pop_obj();
  return res;
}

Token Parser::parse_array() {
  Token tname = parse();
  ASSERTLN(tname.type() == T_NAME || tname.type() == T_RENAME,
           "array: name required");
  Token tsize = parse();
  ASSERTLN(tsize.type() == T_INT, "array: size required");
  int size = tsize.value<int64_t>();
  RnPtr rnp = tname.type() == T_RENAME
                  ? tname.value<RnPtr>()
                  : RnPtr(new Rename(tname.value<string>(), size));
  ASSERTLN(rnp->is_bus, "array: bus required");
  ASSERTLN(size == rnp->bus_size(), "array: size error");
  add_rename(cur_module, "port", rnp);
  parse_ignore();
  return Token(T_ARRAY, rnp);
}

Token Parser::parse_member() {
  Token tok = parse();
  ASSERTLN(tok.type() == T_NAME, "member name required");
  string name = tok.value<string>();
  Token tidx = parse();
  ASSERTLN(tidx.type() == T_INT, "member index required");
  int idx = tidx.value<int64_t>();
  parse_ignore();
  return Token(T_MEMBER, make_pair(name, idx));
}

Token Parser::parse_direction() {
  Token tok = parse();
  ASSERTLN(tok.type() == T_NAME, "direction required");
  string dir = tok.value<string>();
  to_lower(dir);
  parse_ignore();
  return Token(T_DIR, lexical_cast<DirType>(dir));
}

Token Parser::parse_instance() {
  ASSERTLN(cur_module, "instance: no current cell");
  string name = get_name(parse(), cur_module, "instance");
  Token tok = parse();
  ASSERTLN(tok.type() == T_CREF, "instance: cellRef required");
  Module *cell = tok.value<Module *>();
  push_obj(cur_module->create_instance(name, cell));
  Token res = parse_continue();
  pop_obj();
  return res;
}

Token Parser::parse_net() {
  ASSERTLN(cur_module, "net: no current cell");
  string name = get_name(parse(), cur_module, "net");
  push_obj(cur_net = cur_module->create_net(name));
  Token res = parse_continue();
  pop_obj();
  return res;
}

Token Parser::parse_portref() {
  ASSERTLN(cur_net, "portRef: no current net");
  Pin *pin = 0;
  Token tok = parse();
  if (tok.type() == T_MEMBER) {
    auto p = tok.value<std::pair<string, int>>();
    Token tt = parse();
    if (tt.type() == T_RPAR) {
      Rename *rnp = get_rename(cur_module, "port", p.first);
      ASSERTLN(rnp, "portRef: array rename not found");
      Port *grp = cur_module->find_port(rnp->name());
      ASSERTLN(grp, "portRef: port array not found");
      pin = grp->mpin(p.second);
    } else {
      ASSERTLN(tt.type() == T_IREF, "portRef: instanceRef required");
      Instance *inst = tt.value<Instance *>();
      Rename *rnp = get_rename(inst->down_module(), "port", p.first);
      ASSERTLN(rnp, "portRef: array rename not found");
      Port *grp = inst->down_module()->find_port(rnp->name());
      ASSERTLN(grp, "portRef: port array not found");
      string pname = grp->mpin(p.second)->name();
      pin = inst->find_pin(pname);
      parse_ignore();
    }
  } else {
    ASSERTLN(tok.type() == T_NAME, "port name required");
    Token tt = parse();
    if (tt.type() == T_RPAR) {
      string name = get_refname(tok, cur_module, "port");
      pin = cur_module->find_pin(name);
      ASSERTLN(pin, "portRef: port not found");
    } else {
      ASSERTLN(tt.type() == T_IREF, "portRef: instanceRef required");
      Instance *inst = tt.value<Instance *>();
      string name = get_refname(tok, inst->down_module(), "port");
      pin = inst->find_pin(name);
      parse_ignore();
    }
  }
  ASSERTLN(pin, "pin not found");
  pin->connect(cur_net);
  return Token(T_NULL);
}

Token Parser::parse_instanceref() {
  ASSERTLN(cur_module, "instanceRef: no current cell");
  string name = get_refname(parse(), cur_module, "instance");
  Instance *inst = cur_module->find_instance(name);
  ASSERTLN(inst, "instance not found");
  parse_ignore();
  return Token(T_IREF, inst);
}

template <typename T>
inline void set_property(Object *obj, const string &name, const Token &tok) {
  if constexpr (is_same_v<T, string>)
    obj->set_property(create_property<T>(obj->class_id(), name),
                      tok.value<T>());
  else if (obj->class_id() == COS::INSTANCE && name.compare(0, 4, "INIT") == 0)
    // convert the type of INIT* property to string
    obj->set_property(create_property<string>(obj->class_id(), name),
                      to_string(tok.value<T>()));
  else
    obj->set_property(create_property<T>(obj->class_id(), name),
                      tok.value<T>());
}

Token Parser::parse_property() {
  Token tok = parse();
  ASSERTLN(tok.type() == T_NAME, "property name required");
  string name = tok.value<string>();
  tok = parse();
  switch (tok.type()) {
  case T_INT:
    set_property<int64_t>(cur_obj(), name, tok);
    break;
  case T_DOUBLE:
    set_property<double>(cur_obj(), name, tok);
    break;
  case T_QSTR:
    set_property<string>(cur_obj(), name, tok);
    break;
  default:
    break;
  }
  return parse_ignore();
}

Token Parser::parse_e() {
  Token tok = parse();
  ASSERTLN(tok.type() == T_INT, "e: integer required");
  double d = static_cast<double>(tok.value<int64_t>());
  tok = parse();
  ASSERTLN(tok.type() == T_INT, "e: integer required");
  auto e = tok.value<int64_t>();
  for (; e > 0; e--)
    d *= 10.;
  for (; e < 0; e++)
    d *= .1;
  parse_ignore();
  return Token(T_DOUBLE, d);
}

} // namespace EDIF
} // namespace COS

namespace COS {
namespace IO {
class EdifLoader : public Loader {
public:
  EdifLoader() : Loader("edif") {}
  void load(std::istream &istrm);
};

void EdifLoader::load(std::istream &istrm) {
  EDIF::Parser(design(), istrm).parse();
}

static EdifLoader edif_loader;
bool using_edif_loader;

} // namespace IO
} // namespace COS
