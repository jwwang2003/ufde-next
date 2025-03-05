#ifndef USINGVERILOG_H
#define USINGVERILOG_H

namespace COS {
namespace IO {
extern bool using_vl_writer;
inline void using_verilog() { using_vl_writer = true; }
} // namespace IO
} // namespace COS

#endif
