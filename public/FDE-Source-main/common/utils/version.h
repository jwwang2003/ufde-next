#ifndef FDE_VERSION_H_
#define FDE_VERSION_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>

/*
 * FDE Version History:
 * 0.1  	08-04-18 	Original version
 * 0.3  	08-06-30 	FDP250K, Circuit data structure
 * 1.0  	09-02-24	FDP1000K, new netlist data structure
 * 1.5  	09-09-30	FQVR300, ISE Flow
 * 2.0  	10-03-30	FDP3P7, CSP Packing, Timing Driven, DC Flow
 * 2.0.1	10-06-08	Support TBUF and carry chain
 * 2.0.2	10-07-30	Bug fix
 * 2.0.3	10-09-09	Optimize P&R
 * 2.0.4	10-11-12	BRAM and DLL support
 * 3.0  	11-08-08	FDP4, new netlist data structure
 * 3.1		11-12-07	Using RapidXML, Support log and report
 * 3.1.1	12-05-29	Increasment routing
 * 3.1.2	12-07-25	New viewer
 * 3.1.3	13-04-15	Bug fix
 * 3.5		15-10-28	Using C++11
 * 3.5.1				Compile common to static lib
 */

#define FDE_NAME "FDE2018"
#define FDE_VERSION "3.5.1"
#define FDE_VER_NUM 30501
#define FDE_COPYRIGHT                                                          \
  "Copyright(C) 2005-2018. CAD Team, State Key Laboratory of ASIC & System, "  \
  "Fudan University"

inline std::string fde_copyright(const char *prog_name) {
  return boost::str(boost::format("%1% Release %2% -- %3%\n%4%\n") % FDE_NAME %
                    FDE_VERSION % prog_name % FDE_COPYRIGHT);
}

inline std::string timestamp() {
  return boost::posix_time::to_simple_string(
      boost::posix_time::second_clock::local_time());
}

#endif
