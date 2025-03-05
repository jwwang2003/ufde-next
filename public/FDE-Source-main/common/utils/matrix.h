#ifndef FDU_MATRIX_H_
#define FDU_MATRIX_H_

#include "log.h"   // ASSERT
#include "utils.h" // Point

namespace FDU {

//////////////////////////////////////////////////////////////////////////
// Matrix for convenience

template <class T> class Matrix {
  T **c;
  size_t _row, _col;

public:
  Matrix() : c(nullptr), _row(0), _col(0) {}
  Matrix(size_t row, size_t col) { allocate(row, col); }
  ~Matrix() { deallocate(); }

  void allocate(size_t row, size_t col) {
    if (row * col > 0) {
      _row = row;
      _col = col;

      c = new T *[_row];
      for (size_t row = 0; row < _row; ++row)
        c[row] = new T[_col];
    } else {
      _row = _col = 0;
      c = nullptr;
    }
  }

  void deallocate() {
    for (size_t row = 0; row < _row; ++row)
      delete[] c[row];
    delete[] c;
  }

  void renew(size_t row, size_t col) {
    deallocate();
    allocate(row, col);
  }

  Point size() const { return Point(_row, _col); }

  T *operator[](int i) {
    ASSERT(i >= 0 && i < _row, "Access illegal range of Matrix");
    return c[i];
  }

  T &at(int x, int y) {
    ASSERT(x >= 0 && x < _row && y >= 0 && y < _col,
           "Access illegal range of Matrix");
    return c[x][y];
  }

  T &at(const Point &p) {
    ASSERT(p.x >= 0 && p.x < _row && p.y >= 0 && p.y < _col,
           "Access illegal range of Matrix");
    return c[p.x][p.y];
  }

  const T *operator[](int i) const {
    ASSERT(i >= 0 && i < _row, "Access illegal range of Matrix");
    return c[i];
  }

  const T &at(int x, int y) const {
    ASSERT(x >= 0 && x < _row && y >= 0 && y < _col,
           "Access illegal range of Matrix");
    return c[x][y];
  }

  const T &at(const Point &p) const {
    ASSERT(p.x >= 0 && p.x < _row && p.y >= 0 && p.y < _col,
           "Access illegal range of Matrix");
    return c[p.x][p.y];
  }
};

} /* namespace FDU */

#endif /*FDU_MATRIX_H_*/
