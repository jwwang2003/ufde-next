#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include "utils.h"
#include <exception>

///////////////////////////////////////////////////////////////////////////////////////

struct Exception : public std::exception {
  Exception(const std::string &message) throw() : _msg(message) {}
  const char *what() const throw() { return _msg.c_str(); }
  ~Exception() throw() {}

private:
  std::string _msg;
};

struct CilException : public Exception {
  CilException(
      const std::string &msg = "An error occurred during building CFG Lib")
      : Exception("[cil] " + msg) {}
};

struct CktException : public Exception {
  CktException(
      const std::string &msg = "An error occurred during building circuit")
      : Exception("[circuit] " + msg) {}
};

struct BstrException : public Exception {
  BstrException(
      const std::string &msg = "An error occurred during generating bitstream")
      : Exception("[bitstream] " + msg) {}
};

using ccstr = const char *;

//////////////////////////////////////////////////////////////////////////
// Warning

struct Warning {
  std::string _msg;

  explicit Warning(const std::string &msg) : _msg("<Warning> " + msg) {}
  operator ccstr() const { return _msg.c_str(); }
};

// inline std::ostream& operator << (std::ostream& out, const Warning& warning){
//	return out << warning._msg.c_str();
// }

//////////////////////////////////////////////////////////////////////////
// Info

struct Info {
  std::string _msg;

  explicit Info(const std::string &msg) : _msg(msg) {}
  operator ccstr() const { return _msg.c_str(); }
};

// inline std::ostream& operator << (std::ostream& out, const Info& info){
//	return out << info._msg.c_str();
// }

#endif