#ifndef __NLF_ARGS_H__
#define __NLF_ARGS_H__

#include <string>

namespace FDU {
namespace NLF {

using std::string;

struct NLFArgs {
  string _xml;    ///< netlist
  string _cfg;    ///< config lib
  string _lib;    ///< cell lib
  string _output; ///< output file name
  bool _encrypt;  ///< encrypted output?

  void try_parse(int argc, char *argv[]);

protected:
  void check();
  void usage() const;
};

} // namespace NLF
} // namespace FDU

#endif