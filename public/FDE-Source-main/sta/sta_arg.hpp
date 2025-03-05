#ifndef STA_ARG_HPP
#define STA_ARG_HPP

#include <string>

namespace FDU {
namespace STA {

struct STAArg {
  STAArg()
      : grm_name("GSB"), config("STA.conf"), encrypt_out(true),
        dump_spice_for_net(false), dump_dot_for_net(false), num_path(5) {}

  bool parse_command(int argc, char *argv[]);

  static STAArg &instance() {
    static STAArg a;
    return a;
  }

  std::string arch;
  std::string input;
  std::string iclib;
  std::string output;
  std::string report;
  std::string grm_name;

  std::string config;
  bool encrypt_out;
  bool dump_spice_for_net;
  bool dump_dot_for_net;
  int num_path;
};

} // namespace STA
} // namespace FDU

#endif
