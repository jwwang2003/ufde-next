#ifndef _ARGS_H_
#define _ARGS_H_

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

struct Args {
  static const char *_ARCH_PATH;
  static const char *_CIL_PATH;
  static const char *_NETLIST_PATH;
  static const char *_BITFILE_PATH;
  static const char *_LOGFILE_PATH;
  /*	static const char* _FRMFILE_PATH;*/
  static const char *_FORMAT;

  std::string _device;
  std::string _package;
  std::string _arch;
  std::string _cil;
  std::string _netlist;
  std::string _bitstream;
  int _partialbitstream;
  std::string _lib_dir;
  std::string _work_dir;
  std::string _log_dir;
  bool _logSwitch;
  bool _frmSwitch;
  bool _encrypt;
  bool _sopcSwitch;

  void tryParse(int argc, char *argv[]);
  void check();
  void dispHelp() const;
};

extern Args args;

inline std::ostream &operator<<(std::ostream &out, const Args &args) {
  boost::format fm(Args::_FORMAT);
  out << str(fm % "arch   " % args._arch);
  out << str(fm % "cil    " % args._cil);
  out << str(fm % "netlist" % args._netlist);
  out << str(fm % "bstream" % args._bitstream);
  return out;
}

#endif