/**
 * \file lib_util.h
 * \author Zhou Xuegong
 * \date 2007-9-7
 * \brief utilities
 */

#ifndef FUDANFPGA_UTILS_H_
#define FUDANFPGA_UTILS_H_

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <iostream>
#include <string>
#include <vector>

// #include <boost/foreach.hpp>
//  make BOOST_FOREACH easier to use
// #define foreach BOOST_FOREACH

namespace FDU {
using std::size_t;
using std::string;

struct Point {
  int x, y, z;
  Point(int _x = -1, int _y = -1, int _z = -1) : x(_x), y(_y), z(_z) {}

  bool equal2d(const Point &_c) const { return x == _c.x && y == _c.y; }

  bool operator==(const Point &_c) const {
    return x == _c.x && y == _c.y && z == _c.z;
  }
  bool operator!=(const Point &_c) const { return !(operator==(_c)); }
  bool operator<(const Point &_c) const {
    return x < _c.x || (x == _c.x && y < _c.y) ||
           (x == _c.x && y == _c.y && z < _c.z);
  }
};

inline const Point add2d(const Point &p1, const Point &p2) {
  return Point(p1.x + p2.x, p1.y + p2.y);
}

inline const Point operator+(const Point &p1, const Point &p2) {
  return Point(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
}

inline const Point operator-(const Point &p1, const Point &p2) {
  return Point(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
}

inline std::istream &operator>>(std::istream &s, Point &p) {
  int x, y, z = -1;
  char c = 0;

  s >> std::ws >> x >> c;
  if (c == ',')
    s >> std::ws >> y;
  else
    s.setstate(std::ios_base::failbit);

  if (s) {
    c = 0;
    s >> c;
    if (c == ',')
      s >> std::ws >> z;
    s.clear(s.rdstate() & ~std::ios_base::failbit); // clear failbit
    p = Point(x, y, z);
  }

  return s;
}

inline std::ostream &operator<<(std::ostream &s, const Point &p) {
  s << p.x << ',' << p.y;
  if (p.z >= 0)
    s << ',' << p.z;
  return s;
}

///////////////////////////////////////////////////////////////////////////////////////
/// \class EnumStringMap
/// convert between enum and string

template <typename E> class EnumStringMap {
  const char *const *_str_table;
  const int _size;
  const E _def;

public:
  template <typename SA>
  explicit EnumStringMap(const SA &st, E default_value = (E)0)
      : _str_table(st), _size(sizeof(st) / sizeof(char *)),
        _def(default_value) {}

  E getEnum(const string &str) const {
    for (int i = 0; i < _size; ++i)
      if (str == _str_table[i])
        return str.empty() ? _def : static_cast<E>(i);
    throw std::bad_cast();
  }

  const char *getString(E val) const {
    return val == _def && (val < 0 || val >= _size) ? "" : _str_table[val];
  }

  E readEnum(std::istream &s) const {
    string str;
    s >> std::ws;
    if (!s.eof())
      s >> str; // empty input
    return getEnum(str);
  }

  std::ostream &writeEnum(std::ostream &s, E val) {
    return s << getString(val);
  }
};

} /* namespace FDU */

// using members as predicate
// #define mem_cond(fn)		[](const auto* p) { return p->fn; }
// #define mem_cond2(fn1, fn2)	[](const auto* p) { return p->fn1 && p->fn2; }

#endif /*FUDANFPGA_UTILS_H_*/
