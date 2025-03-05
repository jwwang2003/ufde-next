/** \file main.cpp
    \author Chen Zhihui
    \data 2009-4-4

    \brief main file

    This is a technology mapping program for FDP1000K, FPGA Group, Fudan
   University.

    copyright (c) Chen Zhihui 2009
*/

#include "log.h"
#include "mapping.hpp"
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

int main(int argc, char **argv) {
  Args args(argc, argv);
  MappingManager mm(args);

  auto t0 = steady_clock::now();

  FDU_LOG(INFO) << "read netlist";
  mm.doReadDesign();
  auto dur = duration_cast<duration<double>>(steady_clock::now() - t0).count();
  FDU_LOG(INFO) << dur << "s pased.";

  if (args.flow == Args::DC) {
    FDU_LOG(INFO) << "strash Aig";
    mm.doAigTransform();
    auto dur =
        duration_cast<duration<double>>(steady_clock::now() - t0).count();
    FDU_LOG(INFO) << dur << "s pased.";

    FDU_LOG(INFO) << "mapping";
    mm.doMapCut();
    dur = duration_cast<duration<double>>(steady_clock::now() - t0).count();
    FDU_LOG(INFO) << dur << "s pased.";
  }

  FDU_LOG(INFO) << "pattern match";
  mm.doPtnMatch();
  dur = duration_cast<duration<double>>(steady_clock::now() - t0).count();
  FDU_LOG(INFO) << dur << "s pased.";

  FDU_LOG(INFO) << "write netlist";
  mm.doWriteDesign();
  dur = duration_cast<duration<double>>(steady_clock::now() - t0).count();
  FDU_LOG(INFO) << dur << "s pased.";

  FDU_LOG(INFO) << "write report";
  mm.doReport();

  dur = duration_cast<duration<double>>(steady_clock::now() - t0).count(); 
  FDU_LOG(INFO) << "Successfully map the netlist. Elapsed Time: " << setprecision(0) << dur << "s";

  return 0;
}