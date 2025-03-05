#ifndef VL2XMLAPP_H
#define VL2XMLAPP_H

#include "VerilogDesign.h"
#include "netlist.hpp"
#include <boost/program_options.hpp>

namespace VL2XML_PARSER {

/*		this class only provide necessary functions and members to
 * support the user to control the parser		*/
class Vl2xmlApp {
public:
  Vl2xmlApp()
      : _verilogDesign(nullptr), _needClarify(false), _needExportXML(false),
        _needReport(false), _needVerilog(false), _needRename(true),
        _needEncrypt(true)
  //				_topCircuit(new Cell)
  {}
  ~Vl2xmlApp() {
    delete _verilogDesign;
    delete _XMLDesign;
  }
  void initialize();
  int clarifyDesign();
  int ProcessArgs(int argc, char *argv[]);

  // function which call the 3 procedures below
  int parseDesign();

  // parse a sigle verilog file
  int parseSourceFile(string name);
  // read a CellLibrary(just call function is data structure)
  int readLibrary(string libraryFile);
  // parse the instance-mapping-file
  int parseMapFile(string name);

  // transfer data from own D.S to project D.S and call saveXML()
  int doExport();
  // dump data structure into xml file
  int saveXML();
  // output report
  int doReport();
  bool needsDoReport();
  void exitApp(int status);
  void doRename();

  vector<string> _SrcFileList;
  VerilogDesign *_verilogDesign; // only one design at a time containing one or
                                 // more than one files
  // Cell				*_topCircuit;
  // //only one top module
  string _prjName;
  string _XMLFileName;    // XML output
  string _ReportFileName; // Report output
  string _LibraryName;
  string _VerilogFileName; // verilog file name
  bool _needClarify;       // need clarify
  bool _needExportXML;     // need export xml
  bool _needReport;
  bool _needVerilog;
  bool _needRename;
  bool _needEncrypt; // encrypt flag

  Library *cell_lib;  // library
  Design *_XMLDesign; // the design containing work_lib and cell_lib

  // MapInstance*		_MapInstance;
  // //pointer to array of MapInstance
};
} // namespace VL2XML_PARSER
#endif