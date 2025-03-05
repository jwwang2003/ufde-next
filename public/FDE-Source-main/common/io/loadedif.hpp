#ifndef LOADEDIF_HPP
#define LOADEDIF_HPP

#include "netlist.hpp"
#include <boost/any.hpp>
#include <map>

namespace COS {
namespace EDIF {

struct Rename {
  string old_name, new_name;
  bool is_bus;
  int msb, lsb;

  const string &name() const;
  int bus_size() const { return msb > lsb ? msb - lsb + 1 : lsb - msb + 1; }
  Rename(const string &nname, const string &oname);
  Rename(const string &name, int size);
};

enum token_type {
  T_EOF,
  T_LPAR,
  T_RPAR,
  T_QSTR,
  T_INT,
  T_DOUBLE,
  T_NAME,
  T_RENAME,
  T_ERROR,
  T_NULL,
  T_CREF,
  T_LREF,
  T_PREF,
  T_IREF,
  T_DIR,
  T_ARRAY,
  T_MEMBER
};

class Token {
public:
  Token(token_type t = T_ERROR) : _type(t) {}
  template <typename T> Token(token_type t, const T &v) : _type(t), _value(v) {}

  token_type type() const { return _type; }
  template <typename T> void set_value(const T &val) { _value = val; }
  template <typename T> T value() const { return boost::any_cast<T>(_value); }

private:
  token_type _type;
  boost::any _value;
};

class Parser {
public:
  Parser(COS::Design *design, std::istream &is);
  Token parse();

protected:
  typedef Token (Parser::*parse_func)();

  Token parse_ignore();
  Token parse_continue();
  Token parse_rename();
  Token parse_library();
  Token parse_cell();
  Token parse_design();
  Token parse_cellref();
  Token parse_libraryref();
  Token parse_port();
  Token parse_direction();
  Token parse_instance();
  Token parse_net();
  Token parse_portref();
  Token parse_instanceref();
  Token parse_property();
  // Token parse_string();
  // Token parse_integer();
  Token parse_e();
  Token parse_array();
  Token parse_member();

  std::map<string, parse_func> _funcs;
  std::string _cell_lib_old, _cell_lib_new; // cell lib rename

  void push_obj(COS::Object *obj) { obj_stack.push_back(obj); }
  void pop_obj() { obj_stack.pop_back(); }
  COS::Object *cur_obj();

  COS::Design *cur_design;
  COS::Library *cur_lib;
  COS::Module *cur_module;
  COS::Net *cur_net;
  std::vector<COS::Object *> obj_stack;

private:
  int getc();
  void ungetc(char ch);
  Token get_token();
  std::string get_name(const Token &tok, const COS::Object *owner,
                       const std::string &type) const;
  std::string get_refname(const Token &tok, const COS::Object *owner,
                          const std::string &type) const;

  std::istream &_source; // file to parse from
  int _line;             // line number in file
                         //		bool _token_pushed;
};

} // namespace EDIF
} // namespace COS

#endif
