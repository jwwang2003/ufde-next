#ifndef USINGRTSIMVL_H
#define USINGRTSIMVL_H

namespace COS {
namespace IO {
extern bool using_rtsimvl_writer;
inline void using_rtsimvl() { using_rtsimvl_writer = true; }
} // namespace IO
} // namespace COS

#endif
