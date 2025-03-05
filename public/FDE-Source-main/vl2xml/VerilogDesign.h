/* (c) Copyright 2004-2005, Cadence Design Systems, Inc.  All rights reserved.

This file is part of the OA Gear distribution.  See the COPYING file in
the top level OA Gear directory for copyright and licensing information. */

/*
Author: Aaron P. Hurst <ahurst@eecs.berkeley.edu>

ChangeLog:
2005-05-31: ChangeLog started
*/

#ifndef VERILOGDESIGN_H
#define VERILOGDESIGN_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include "module.h"

namespace VL2XML_PARSER {

using std::list;
using std::string;
using std::vector;
using namespace COS;
// *****************************************************************************
// VerilogDesign
//
/// \brief An entire Verilog design.
//
// *****************************************************************************

class VerilogDesign {

public:
  VerilogDesign() : _topModule(nullptr) {}

  int initializeGlobal();
  int clarifyDesign(Library *cell_lib);
  int clarifyPorts();
  int clarifyNames();
  int clarifyModules(Library &cell_lib);
  int clarifyInstanceType(
      Library *cell_li); //$$$$$$$$need more attetion$$$$$$$$$$----function same
                         // with the doconvert
  int clarifyInstanceConnection();
  int DataStructureConvertion(Design *design, Library &cell_lib);
  void RegisterModules();

  ~VerilogDesign();

  /// If DELETE_UPON_DESTRUCTION is set true,
  /// the destructor of a VerilogDesign will
  /// deallocate all of the memory in the subcomponents.

  /// some members need to be declared private
  vector<string> _sourceList;
  string _OutXMLname;
  string _OutVerilogName;
  // map<string, int>			_ParameterMap;

  list<VerilogModule *> modules;
  VerilogModule *_topModule; // not the same with the App's member "_topModule"

  // static const bool DELETE_UPON_DESTRUCTION = true;
};
} // namespace VL2XML_PARSER

#endif
