#ifndef USINGSIMVL_H
#define USINGSIMVL_H

namespace COS {
namespace IO {
extern bool using_simvl_writer;
inline void using_simvl() { using_simvl_writer = true; }
} // namespace IO
} // namespace COS

#endif
