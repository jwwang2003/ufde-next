#ifndef PLCARGS_H
#define PLCARGS_H

#include "plc_utils.h"

namespace FDU {
namespace Place {

using std::string;

struct PlaceArgs {
  string _arch_lib;
  string _library;
  string _input_nl;
  string _output_nl;
  string _delay_lib;
  string _plc_csts;
  string _jtag_csts;
  PLACER::EffortLevel _effort_level;
  PLACER::PlaceMode _mode;
  bool _update_rand_seed;
  bool _encrypt;

  PlaceArgs()
      : _arch_lib(""), _library(""), _input_nl(""), _output_nl(""),
        _delay_lib(""), _plc_csts(""), _jtag_csts(""),
        _effort_level(PLACER::STANDARD), _mode(PLACER::BOUNDING_BOX),
        _update_rand_seed(false), _encrypt(false) {}
  void try_parse(int argc, char *argv[]);

protected:
  void check() const;
  void disp_help() const;
};

} // namespace Place
} // namespace FDU

#endif