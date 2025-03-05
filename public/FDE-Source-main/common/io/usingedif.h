#ifndef USINGEDIF_H
#define USINGEDIF_H

namespace COS {
namespace IO {
extern bool using_edif_loader;
inline void using_edif() { using_edif_loader = true; }
} // namespace IO
} // namespace COS

#endif
