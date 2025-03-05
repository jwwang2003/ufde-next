#include <fstream>
#include <iostream>

#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "log.h"
#include "utils.h" // EnumStringMap

namespace FDU {
namespace LOG {

using namespace std;
using namespace boost;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

static const char *sev_levels[] = {"DEBUG", "VERBOSE", "INFO ", "WARN ",
                                   "ERROR"};
static FDU::EnumStringMap<severity_level> levelmap(sev_levels);
std::ostream &operator<<(std::ostream &s, severity_level level) {
  return levelmap.writeEnum(s, level);
}

Log &Log::get() {
  static Log instance;
  return instance;
}

Log::Log() : core_(logging::core::get()) {
  sink_con_ = logging::add_console_log(
      std::clog,
      keywords::format = expr::format("%1%: %2%") %
                         expr::attr<severity_level>("Severity") % expr::message,
      keywords::filter = expr::attr<severity_level>("Severity") >= INFO);
}

void Log::init_log_file(const std::string &log_file,
                        const std::string &prog_name) {
  core_->add_global_attribute("TimeStamp", attrs::local_clock());
  core_->add_global_attribute("ProgName",
                              attrs::constant<std::string>(prog_name));
  core_->remove_sink(sink_file_);     // remove old sink
  sink_file_ = logging::add_file_log( // add new sink
      log_file, keywords::open_mode = std::ios_base::app,
      keywords::auto_flush = true,
      keywords::format = expr::format("[%1%] %2%: %3%: %4%") %
                         expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d, %H:%M:%S.%f") %
                         expr::attr<std::string>("ProgName") %
                         expr::attr<severity_level>("Severity") %
                         expr::message);
}

} // namespace LOG
} // namespace FDU
