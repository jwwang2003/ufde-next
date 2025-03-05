#ifndef USINGCHECK_H
#define USINGCHECK_H

namespace COS {
namespace IO {
extern bool using_chk_writer;
inline void using_check() { using_chk_writer = true; }
} // namespace IO
} // namespace COS

#endif
