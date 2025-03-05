#ifndef _SIZESPAN_H_
#define _SIZESPAN_H_

#include <iostream>
#include <sstream>
#include <string>

struct sizeSpan {
  int _rowSpan;
  int _columnSpan;

  sizeSpan(int rowSpan = 1, int columnSpan = 1)
      : _rowSpan(rowSpan), _columnSpan(columnSpan) {}

  bool isValid() const { return _rowSpan != -1 && _columnSpan != -1; }

  static void extractCoordinate(const char *coordinate, char xAxis, int &x,
                                char yAxis, int &y);

  // sizeSpan &operator=(const sizeSpan &t) {
  //   _rowSpan = t._rowSpan;
  //   _columnSpan = t._columnSpan;
  //   return *this;
  // }
};

inline bool operator==(const sizeSpan &lhs, const sizeSpan &rhs) {
  return lhs._rowSpan == rhs._rowSpan && lhs._columnSpan == rhs._columnSpan;
}

inline bool operator!=(const sizeSpan &lhs, const sizeSpan &rhs) {
  return !(lhs == rhs);
}

inline std::istream &operator>>(std::istream &s, sizeSpan &span) {
  std::istream::iostate oldState = s.rdstate();
  int rowSpan = -1, colSpan = -1;
  char c = 0;

  if (s >> std::ws >> rowSpan >> c && (c == 'R' || c == 'B')) {
    if (s >> colSpan >> c && (c == 'C' || c == 'W'))
      span = sizeSpan(rowSpan, colSpan);
    else
      span = sizeSpan(-1, -1);
    return s;
  }

  s.clear(oldState);

  if (s >> std::ws >> c >> rowSpan && (c == 'R' || c == 'B')) {
    if (s >> c >> colSpan && (c == 'C' || c == 'W'))
      span = sizeSpan(rowSpan, colSpan);
    else
      span = sizeSpan(-1, -1);
  } else
    span = sizeSpan(-1, -1);

  return s;
}

inline std::ostream &operator<<(std::ostream &s, const sizeSpan &span) {
  s << 'R' << span._rowSpan << 'C' << span._columnSpan;
  return s;
}

#endif