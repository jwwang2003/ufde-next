#ifndef ARGS_HPP
#define ARGS_HPP

#include <string>
using std::string;

struct Args {
  enum FlowType { DC, Yosys };
  string prjName;
  string inputFile;
  string inputType;
  string outputFile;
  string cellLib;
  string verilogFile;
  FlowType flow = DC;
  bool fEncry = true;
  int lutSize = 4; // mapped lut size

  Args(int argc, char **argv);
};

#endif
