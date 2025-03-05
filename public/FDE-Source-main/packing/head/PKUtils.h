#ifndef _PKUTILS_H
#define _PKUTILS_H

#include <boost/algorithm/string.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/regex.hpp>

#include "PKConfig.h"
#include "utils.h"
#include <deque>
#include <list>

namespace PACK {
using namespace FDU;

struct NameInfo {
  string name;
  bool is_fake;
  bool is_kw;

  NameInfo() : is_fake(false), is_kw(false) {}
};

template <typename T> struct DummyCompare {
  bool operator()(const T &, const T &) const { return false; }
};
// simulate set
template <typename T, typename P = DummyCompare<T>>
class unique_list : public std::list<T> {
  typename std::list<T>::iterator locate(const T &v) {
    P comp;
    typename std::list<T>::iterator p = std::list<T>::begin();
    for (; p != std::list<T>::end() && comp(*p, v); ++p)
      ;
    for (; p != std::list<T>::end() && !comp(v, *p); ++p)
      if (v == *p)
        break;
    return p;
  }

public:
  typedef std::list<T> base;
  unique_list() : base() {}
  explicit unique_list(const base &lst) : base(lst) {}

  typename std::list<T>::iterator find(const T &v) {
    return std::find(std::list<T>::begin(), std::list<T>::end(), v);
  }
  typename std::list<T>::const_iterator find(const T &v) const {
    return std::find(std::list<T>::begin(), std::list<T>::end(), v);
  }

  using base::erase;
  using base::insert;
  typename std::list<T>::iterator insert(const T &v) {
    typename std::list<T>::iterator p = locate(v);
    if (p != std::list<T>::end() && v == *p)
      return p;
    return insert(p, v);
  }
  template <typename IT> void insert(IT first, IT last) {
    while (first != last)
      insert(*first++);
  }
  void erase(const T &v) {
    typename std::list<T>::iterator p = locate(v);
    if (p != std::list<T>::end() && v == *p)
      erase(p);
  }
};

template <typename IT> class Enumeration {
public:
  typedef typename boost::iterator_value<IT>::type val_type;
  typedef typename boost::iterator_pointer<IT>::type ptr_type;
  typedef typename boost::iterator_reference<IT>::type ref_type;

  Enumeration() {}

  Enumeration(IT begin, IT end) : begin_(begin), end_(end), present_(begin) {}

  Enumeration(const Enumeration<IT> &e)
      : begin_(e.begin_), end_(e.end_), present_(e.present_) {}

  Enumeration<IT> &operator=(const Enumeration<IT> &e) {
    begin_ = e.begin_;
    end_ = e.end_;
    present_ = e.present_;
    return *this;
  }

  bool has_more_elem() const { return present_ != end_; }

  ref_type ref_of_next_elem() {
    ASSERT(present_ != end_, "bad iterator");
    return *(present_++);
  }
  val_type val_of_next_elem() {
    ASSERT(present_ != end_, "bad iterator");
    return *(present_++);
  }
  ptr_type ptr_of_next_elem() {
    ASSERT(present_ != end_, "bad iterator");
    return &*(present_++);
  }

private:
  IT begin_, end_;
  IT present_;
};

typedef const string *cstr_ptr;
typedef const char *c_str_ptr;
typedef std::vector<std::string> StrVec;

StrVec &split_statement(const string &, StrVec &);
StrVec &split_params(const string &, StrVec &);
StrVec &split_specific(const string &s, StrVec &svec, const string &delim);

bool match_fake_name(const string &, boost::smatch &);
bool is_fake_name(const string &);

template <typename T, typename C>
bool is_in(const T &elem, const C &container) {
  return find(container.begin(), container.end(), elem) != container.end();
}

string hex2expr(const string &, int);
string exchange_expr(string, const string &, const string &);
string sub_expr(string, const string &, const string &);

} // namespace PACK

#endif
