#include "Vl2xmlApp.h"
#include "DebugHelper.hpp"
#include "version.h"
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

#define USAGE                                                                  \
  "Usage: import {infile1 [infile2] [infile3]...} -v -x [out_xml_file] -c -h"
#define print_usage(s)                                                         \
  {                                                                            \
    std::cout << (s) << std::endl << desc << std::endl;                        \
    return 1;                                                                  \
  }

using VL2XML_PARSER::VerilogDesign;

using namespace COS;
using std::ofstream;
VerilogDesign *yydesign; // can't put that in the VL2XML_PARSER because it be
                         // become localized to the yyparse. Just get the
                         // VerilogDesign out of the namespace.
extern FILE *yyin;
extern char *yyfilename;
extern int yyparse();

namespace VL2XML_PARSER {

namespace po = boost::program_options;
using boost::replace_all;
using boost::property_tree::ptree;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

static string TrimEscapedStr(Module &cell, string escaped, int type) {
  string tmp = escaped.substr(1, escaped.find_last_not_of(' '));
  replace_all(tmp, " ", "_");

  /* If type = 1, then it's a net name ; If type = 2, then it's a instance name
   */
  if (type == 1) {
    while (cell.nets().find(tmp)) {
      tmp = "_" + tmp;
    }
  } else if (type == 2) {
    while (cell.instances().find(tmp)) {
      tmp = "_" + tmp;
    }
  }
  return tmp;
}

void Vl2xmlApp::initialize() {
  /*do some initialization job*/
  _verilogDesign = new VerilogDesign;
}

int Vl2xmlApp::ProcessArgs(int argc, char *argv[]) {
  try {
    // char mode = 0;
    vector<string> s;
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "fde,f", po::value<string>(),
        "set fde file")("celllib,l", po::value<string>(), "set cell library")(
        "input,i", po::value<vector<string>>(),
        "input file(s) to be parsed") /*need to be modified to support multiple
                                         file input*/
        ("check,c", "check names and types")("xml,x", po::value<string>(),
                                             "export xml file")(
            "report,r", "export report file")("verilog,v", "verilog file")(
            "no_rename,N", "don't rename instances and nets")(
            "noencrypt,e", "load and save without encrypt");
    po::positional_options_description p;
    p.add("input", -1);
    po::variables_map vm;
    // save the original.now it is supposed to support the positional arguments
    // po::store(po::parse_command_line(argc, argv, desc), vm);
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    cout << fde_copyright("Import");

    if (vm.count("help")) {
      // print_usage(USAGE);
      cout << desc << endl;
      exit(0);
    }

    if (vm.count("fde")) {
      string _fde_file = vm["fde"].as<string>();
      ptree prj;
      read_info(_fde_file, prj);
      _prjName = prj.get<string>("project.name");
      _LibraryName = prj.get<string>("library.celllib");
    } else {
      // cerr << "Command Line Error: no fde file" << endl;
      // cerr << desc << endl;
      // exit(-1);
    }

    if (vm.count("celllib")) {
      _LibraryName = vm["celllib"].as<string>();
      cout << "Cell library:  " << _LibraryName << endl;
    }

    if (vm.count("input")) {
      s = vm["input"].as<vector<string>>();
      for (vector<string>::const_iterator it = s.begin(); it != s.end(); ++it) {
        if (it->length() > 0) {
          _SrcFileList.push_back(*it);

        } else {
          std::cout << "ERROR : No source file to parse!" << std::endl;
          exit(1);
        }
      }
    }
    //  else {
    //   // _SrcFileList.push_back(_prjName.append(".v"));
    //   std::cout << "ERROR : No source file to parse!" << std::endl;
    //   return 1;
    // }

    if (vm.count("check"))
      _needClarify = true;

    if (vm.count("report")) {
      _ReportFileName = _prjName.append("_trans.rpt");
      _needReport = true;
    }
    if (vm.count("xml")) {
      _XMLFileName = vm["xml"].as<string>();
      _needExportXML = true;
    } else {
      _XMLFileName = _prjName.append("_trans.xml");
      _needExportXML = true;
    }

    if (vm.count("verilog")) {
      _VerilogFileName = _prjName.append("_trans.v");
      _needVerilog = true;
    }
    if (vm.count("no_rename")) {
      _needRename = false;
    }
    if (vm.count("noencrypt")) {
      _needEncrypt = false;
    }

    /*else
            print_usage(USAGE);*/

  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int Vl2xmlApp::parseDesign() {
  std::cout << "Parse Design Start" << std::endl;
  /* 1 pass the design to yacc*/
  yydesign = _verilogDesign;

  /* 2 create a new design in data structure */
  _XMLDesign = new Design("XMLDesign");

  /* 3 read the library*/
  if (readLibrary(_LibraryName)) {
    std::cout << "An error happened in reading cell library!" << std::endl;
    return 1;
  }
  /* 4 read the source files*/
  for (vector<string>::iterator it = _SrcFileList.begin();
       it != _SrcFileList.end(); ++it) {
    parseSourceFile((*it).c_str());
    std::cout << "Parse " << *it << " Done" << std::endl;
  }
  std::cout << "Parse Design Done" << std::endl;
  return 0;
}

int Vl2xmlApp::readLibrary(string libraryFile) {
  /*create(new) the cell_lib from cmd line */

  // 250k-II data structure
  // library_ptr lib = create_library(libraryFile);
  //_XMLDesign->add_library(lib);
  // cell_lib = _XMLDesign->libs().find("cell_lib");//maybe later the name in
  // the find should change to a variable

  // 1000k data structure
  _XMLDesign->load("xml", libraryFile);
  cell_lib = _XMLDesign->find_or_create_library("cell_lib");
  return 0;
}

int Vl2xmlApp::parseSourceFile(std::string name) {
  // extern void initializeParser(const char* name);
  try {
    /*specify the lex input*/
    // initializeParser(name.c_str());
    yyfilename = const_cast<char *>(name.c_str());
    yyin = fopen(yyfilename, "r");

    /*call the yacc-like parser*/
    yyparse();
    fclose(yyin);

  } catch (std::exception &e) {
    /*leave for future improvement*/
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int Vl2xmlApp::parseMapFile(std::string name) {
  /*
   * Assuming the file format is down below:
   *
   *
   *
   *
   *
   *
   *
   *
   *
   **/
  return 0;
}
int Vl2xmlApp::clarifyDesign() {
  if (_needClarify) {
    if (_verilogDesign->clarifyDesign(cell_lib))
      return 1;
  }
  return 0;
}

int Vl2xmlApp::doExport() {
  if (_needExportXML) {
    PUSH_STEP;
    /* 1 convert the raw data into the data structure */
    std::cout << "Start Converting......" << std::endl << std::endl;

    /* 2 convert the design */
    Library &lib = *cell_lib;
    if (_verilogDesign->DataStructureConvertion(_XMLDesign, lib))
      return 1;

    /* 3 change the design name to top module name and flatten the design*/
    _XMLDesign->rename(_XMLDesign->top_module()->name());

    /* Rename instance and net */
    /* Write netlist before flatten() and rename() */
    string base_name = _XMLFileName.substr(0, _XMLFileName.find_first_of('.'));
    string name_before_flatten = base_name + "_before_flatten_rename.xml";

    if (_needRename) {
      doRename();
    }

    // save the netlist before it's flatten
    //_XMLDesign->save(fileio::xml, name_before_flatten);

    _XMLDesign->top_module()->flatten();

    /* 4 save XML */
    std::cout << "Start Saving XML......" << std::endl;
    if (saveXML())
      return 1;

    if (_needVerilog) {
      // 250-II data structure
      //_XMLDesign->work_lib().write_verilog(std::ofstream(_VerilogFileName.c_str()));

      // 1000k data structure
      ofstream output_verilog(_VerilogFileName.c_str(), ofstream::out);
      _XMLDesign->save("verilog", output_verilog);
    }
    POP_STEP;
  }
  return 0;
}
void Vl2xmlApp::doRename() {
  /* For the cell lib */
  Library *cell_lib = _XMLDesign->libs().find("cell_lib");
  for (Module *cell : cell_lib->modules()) {
    int cell_num = cell_lib->modules().size();
    string cell_name = cell->name();
    for (Instance *inst : cell->instances()) {
      string inst_name = inst->name();
      if (inst_name[0] == '\\') {
        inst_name = TrimEscapedStr(*cell, inst_name, 2);
        inst->rename(inst_name);
      }
    }
    for (Net *net : cell->nets()) {
      string net_name = net->name();
      if (net_name[0] == '\\') {
        net_name = TrimEscapedStr(*cell, net_name, 1);
        net->rename(net_name);
      }
    }
  }
  /* For the work lib */
  for (Module *cell : _XMLDesign->work_lib()->modules()) {
    int cell_num = _XMLDesign->work_lib()->modules().size();
    string cell_name = cell->name();

    for (Instance *inst : cell->instances()) {
      string inst_name = inst->name();

      if (inst_name[0] == '\\') {
        inst_name = TrimEscapedStr(*cell, inst_name, 2);
        inst->rename(inst_name);
      }
    }
    for (Net *net : cell->nets()) {
      string net_name = net->name();
      if (net_name[0] == '\\') {
        net_name = TrimEscapedStr(*cell, net_name, 1);
        net->rename(net_name);
      }
    }
  }
}
int Vl2xmlApp::saveXML() {
  PUSH_STEP;
  try {
    PUSH_STEP_NAMED("Consistency Check");
    std::cout << "Begin consistency check......\n";
    std::ostringstream logger;
    _XMLDesign->save("check", logger);

    if (logger.str() != "") {
      std::cout << "consistency_check failed!\n"
                << "The reason is: \n";
      std::cout << logger.str();
      // ReportError(logger.str(), 2);
    } else
      std::cout << "consistency_check succeed!\n";

      //////////////////////////////////////////////////////////////////////////
      // seperate combinational and sequentian netlist by file name
#if 0		
		if (!(_XMLDesign->top_cell().ports().find("clk") || _XMLDesign->top_cell().ports().find("CLK") || _XMLDesign->top_cell().ports().find("CLOCK")))
		{
			std::cout << _XMLFileName << std::endl;
			_XMLFileName = "combinational_" + _XMLFileName;
			std::cout << _XMLFileName << std::endl;
		}
		else{
			_XMLFileName = "sequential_" + _XMLFileName;
		}
#endif
    // save the xml
    _XMLDesign->save("xml", _XMLFileName, _needEncrypt);
    // FDU_LOG(INFO) << "Save XML Successfully!";
    std::cout << "INFO : Successfully save XML file." << std::endl;
  } catch (std::exception &e) {
    std::cout
        << "An error happened in saving xml! The reasong is listed below :"
        << std::endl;
    std::cout << e.what() << std::endl;
    return 1;
  }
  POP_STEP;
  return 0;
}

void Vl2xmlApp::exitApp(int status) {
  string message = "Exit";
  if (status) {
    // errorParse(parserGlobal->getCurrentFile(),parserGlobal->getCurrentLineNo());
    // printLog("Exit with error!", status);
  } else {
    // printLog("Accomplished and exit!");
  }
  exit(status);
}

int Vl2xmlApp::doReport() { return 0; }

} // namespace VL2XML_PARSER