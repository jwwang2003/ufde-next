#ifndef MODULE_H
#define MODULE_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include "VerilogMisc.h"
#include "netlist.hpp"

namespace VL2XML_PARSER {

// *****************************************************************************
// Module
//
/// \brief A single Verilog module
//
/// This module may spawn several OpenAccess modules if multiple
/// parameterizations exist.
//
// *****************************************************************************
class VerilogDesign;

using namespace COS;
using std::list;
using std::map;
using std::string;

class VerilogModule {

public:
  string name;
  bool IsTopModule;
  bool IsConverted;

  VerilogDesign *design;
  list<VPort *> *ports;
  list<Declaration *> declarations;
  list<Declaration *> parameters;
  list<Declaration *> *parameterOverrides;
  list<Assignment *> *assignments;
  list<AlwaysBlock *> *alwaysBlocks;
  list<Statement *> *initialBlocks;
  list<Function *> *functions;
  list<Instantiation *> *instantiations;
  map<string, int> _ParameterMap;
  list<Attribute *> *attributes; // used for storing the comment info

  VerilogModule();
  // Destructor
  ~VerilogModule();
  int clarifyPorts();
  int clarifyAllNames();
  int clarifyInstanceNames();
  int clarifyNetNames();
  int clarifyPortNames();
  int clarifyInstanceType(list<VerilogModule *> &modulelist, Library *cell_lib);
  int clarifyInstanceConnection(const list<VerilogModule *> &modulelist);

  int ConvertModules(Module &cell, Library &cell_lib,
                     list<VerilogModule *> &modules);
  int ConvertNets(Module &cell, Library &cell_lib, int &lineno);
  int ConvertInstances(Module &cell, Library &cell_lib,
                       list<VerilogModule *> &modules, int &lineno);
  int ConvertPorts(Module &cell, Library &cell_lib, int &lineno);
  int ConvertParameters(Module &cell, Library &cell_lib, int &lineno);
  int ConvertAssignments(Module &cell, Library &cell_lib, int &lineno);
  int ConvertAttributes(Module &cell, Library &cell_lib, int &lineno);
  int ConvertParameterOverrides(Module &cell, Library &cell_lib, int &lineno);

private:
  void ParsePrimaryNet(Primary *prim, list<string> &namelist);
  void ParsePrimaryNet(string name, Expression *start, Expression *stop,
                       list<string> &namelist, int &rstart, int &rstop);
  void ParseBundleNet(Bundle *bundle, list<string> &namelist, Module &cell,
                      Library &cell_lib);
  void PrimaryOrBundle(Expression *expr, list<string> &namelist, Module &cell,
                       Library &cell_lib);
  // void ParseConst();

  void ConnectInstance(Module &cell, Instance &inst, list<string> &namelist,
                       string portname);

  void MergeNets(map<string, int> *eg, Module &cell, int no);
};

} // namespace VL2XML_PARSER

#endif