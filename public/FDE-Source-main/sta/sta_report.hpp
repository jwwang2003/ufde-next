#ifndef STA_REPORT_HPP
#define STA_REPORT_HPP

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>

// unoptimized, copy from old version

namespace FDU {
namespace STA {

namespace STA_RPT_FORMAT {
const std::string FULL_DELIM = "-----------------------------------------------"
                               "-----------------------------";

const std::string HALF_DELIM = "-------------------------------";
} // namespace STA_RPT_FORMAT

class STANode;
class STAEngine;
class STAReport {
public:
  enum REPORT_MODE { R2R, R2O, I2R };

  explicit STAReport(STAEngine *e) : _sta_engine(e) {}

  // void export_report(const std::string& fname, int num_paths, REPORT_MODE
  // mode = R2R);
  void export_report(const std::string &fname, int num_paths,
                     REPORT_MODE mode = I2R);

protected:
  typedef std::multimap<double, std::list<STANode *>> PathSort;

  void report_title(std::ofstream &rpt) const;
  void report_r2r(std::ofstream &rpt, int num_paths) const;
  void report_r2o(std::ofstream &rpt, int num_paths) const;
  void report_i2r(std::ofstream &rpt, int num_paths) const;

  // for all modes
  void report_paths(std::ofstream &rpt, PathSort &paths, bool rise,
                    double cur_domain_max_delay, int cur_domain,
                    int num_paths) const;

private:
  STAEngine *_sta_engine;
};

//////////////////////////////////////////////////////////////////////////
// io streams

std::istream &operator>>(std::istream &s, STAReport::REPORT_MODE &mode);
std::ostream &operator<<(std::ostream &s, STAReport::REPORT_MODE mode);

} // namespace STA
} // namespace FDU

#endif
