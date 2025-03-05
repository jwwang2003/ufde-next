#ifndef FDU_LOGGING_H_
#define FDU_LOGGING_H_

#include <iosfwd>
#include <stdexcept>
#include <string>

#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/noncopyable.hpp>

namespace FDU {
struct AssertionFail : std::runtime_error {
  AssertionFail()
      : std::runtime_error("Assertion failed, please check the log file for "
                           "more information.") {}
};

namespace LOG {
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;

enum severity_level { DEBUG, VERBOSE, INFO, WARN, ERR };
std::ostream &operator<<(std::ostream &s, severity_level level);

using sev_logger = src::severity_logger<severity_level>;
using sp_log_core = boost::shared_ptr<logging::core>;
using sp_file_sink =
    boost::shared_ptr<sinks::synchronous_sink<sinks::text_file_backend>>;
using sp_stream_sink =
    boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>>;

class Log : boost::noncopyable {
public:
  void init_log_file(const std::string &log_file, const std::string &prog_name);

  sev_logger &logger() { return slg_; }
  sp_stream_sink con_sink() { return sink_con_; }
  sp_file_sink file_sink() { return sink_file_; }

  static Log &get(); // get the singleton instance

private:
  Log();

  sp_log_core core_;
  sev_logger slg_;
  sp_stream_sink sink_con_;
  sp_file_sink sink_file_;
};

inline sev_logger &get_logger() { return Log::get().logger(); }

inline void init_log_file(const std::string &log_file,
                          const std::string &prog_name = "") {
  Log::get().init_log_file(log_file, prog_name);
}

template <typename SP, typename FMT>
inline void set_format(SP sink, FMT const &fmt) {
  sink->set_formatter(fmt);
}

template <typename F> inline void set_console_format(F const &fmt) {
  set_format(Log::get().con_sink(), fmt);
}
template <typename F> inline void set_file_format(F const &fmt) {
  set_format(Log::get().file_sink(), fmt);
}

} // namespace LOG
} // namespace FDU

#define FDU_LOG(level) BOOST_LOG_SEV(FDU::LOG::get_logger(), FDU::LOG::level)
#define FDU_LOGV(level) BOOST_LOG_SEV(FDU::LOG::get_logger(), level)

#define ASSERT(asst, msg)                                                      \
  if (!(asst)) {                                                               \
    FDU_LOG(ERR) << __FILE__ << '(' << __LINE__ << "): " << msg;               \
    throw FDU::AssertionFail();                                                \
  }

#define ASSERTS(asst) ASSERT(asst, "Assertion " #asst " failed.")

#ifdef _DEBUG
#define FDU_DEBUG(msg) FDU_LOG(DEBUG) << msg
#define ASSERTD ASSERT
#define ASSERTSD ASSERTS
#else
#define FDU_DEBUG(msg)
#define ASSERTD(a, m)
#define ASSERTSD(a)
#endif

#endif // FDU_LOGGING_H_
