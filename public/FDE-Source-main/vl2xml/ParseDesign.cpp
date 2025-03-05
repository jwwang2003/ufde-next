

#include "DebugHelper.hpp"
#include "VerilogDesign.h"
#include "module.h"
#include <exception>

#define DELETE_UPON_DESTRUCTION 1

// #define _BUILD_CELLLIBRARY2_0

namespace VL2XML_PARSER {
// using namespace DebugHelper;

VerilogDesign::~VerilogDesign() {
  if (!DELETE_UPON_DESTRUCTION)
    return;

  for (list<VerilogModule *>::iterator it = modules.begin();
       it != modules.end(); ++it) {
    delete (*it);
  }
}

// int	VerilogDesign::ParseDesign(vector<string> srcList, string outXMLname,
// string outVerilogname, string libraryName)
//{
//	_sourceList = srcList;
//	_OutXMLname = outXMLname;
//	_OutVerilogName = outVerilogname;
//
//	/*read the cell lib*/
//	readLibrary(libraryName);

//	for (vector<string>::iterator it = _sourceList.begin();it !=
//_sourceList.end();++it)
//	{
//		ParseSourceFile((*it).c_str());
//	}
//	return 0;
//}

// int VerilogDesign::ParseSourceFile(std::string name){

//	extern void initializeParser(const char* name);
//	extern int  yyparse();

//	try
//	{
//		/*specify the lex input*/
//		initializeParser(name.c_str());

//		/*call the yacc-like parser*/
//		yyparse();

//	}
//	catch (exception* e)
//	{
//		/*leave for future improvement*/
//	}
//	return 0;
//}

// int VerilogDesign:: readLibrary(string libraryFile){
//	/*create(new) the cell_lib from cmd line */
//	cell_lib = create_library(libraryFile);

//	return 0;
//}

int VerilogDesign::initializeGlobal() {
  return 0;
  /*no use now*/
}

int VerilogDesign::DataStructureConvertion(Design *design, Library &cell_lib) {
  PUSH_STEP;
  /* create(new) and add the work_lib to the design */
  Library *work_lib = design->create_library("work_lib");

  /* find the top module or simply specified by user */
  if (!1) // user specified the top module#########need to provide the interface
  {
  } else
    clarifyModules(cell_lib);

  /* Here Only convert the top module! */
  for (list<VerilogModule *>::iterator it = modules.begin();
       it != modules.end(); it++) {
    if ((*it)->IsTopModule) {
      /*create one cell for every module using the same cell_lib*/
#ifndef _BUILD_CELLLIBRARY2_0
      Module *newcell = work_lib->create_module((*it)->name, "GENERIC");

      design->set_top_module(newcell);
      (*it)->ConvertModules(*newcell, cell_lib, modules);
#else
      Module *newcell =
          cell_lib->create_module((*it)->name, "GENERIC"); // for library
      design->set_top_module(newcell);
      (*it)->ConvertModules(*newcell, *cell_lib, modules);
#endif
      /*#######there is some problem here.When a verilog file like the one with
      just port and module name in it ########
      #########i can't put the module in the cell lib.so the program is not
      general enough right now like modelSim######*/
    }
  }
  for (list<VerilogModule *>::iterator it = modules.begin();
       it != modules.end(); it++) {
    if ((*it)->IsConverted)
      continue;
    else {
      std::cout << "Warning : Module : " << (*it)->name << " is not used!\n"
                << std::endl;
    }
  }
  PUSH_STEP;
  return 0;
}

// int VerilogDesign::saveXML(Design* design){
//	/*save the design into xml*/
//	design->save_xml(_OutXMLname);

//	/*save the cell_lib, work_lib into xml  (optional)*/
//	return 0;
//}

int VerilogDesign::clarifyDesign(Library *cell_lib) {
  /*  <0> no use now */
  clarifyPorts();

  /*  <1> find if there is any undefined module which is instantiated from    */
  clarifyInstanceType(cell_lib);

  /*  <2> find if there is repeated names for instances,wires,ports */
  /*      no use now */
  clarifyNames();

  /*  <3> identify the top module */
  /*  should be put out of the clarify function because it is a compusalry
   * function for the whole parser*/

  /*	<4> find if the instance ports connection is conformed with the
   * library*/
  /*		no use now */
  clarifyInstanceConnection();

  return 0;
}

int VerilogDesign::clarifyPorts() {
  /*for(list<Module *>::iterator it=modules.begin(); it != modules.end(); it++)
  {
          (*it).clarifyPorts();
  }*/
  return 0;
}

int VerilogDesign::clarifyInstanceType(Library *cell_lib) {
  for (list<VerilogModule *>::iterator it = modules.begin();
       it != modules.end(); it++) {
    (*it)->clarifyInstanceType(modules, cell_lib);
  }
  return 0;
}

int VerilogDesign::clarifyNames() {

  for (list<VerilogModule *>::iterator it = modules.begin();
       it != modules.end(); it++) {
    (*it)->clarifyAllNames();
  }
  return 0;
}

int VerilogDesign::clarifyModules(Library &cell_lib) {
  /* using the simple assumption that a module which is never instantiated is
   * the top module*/
  /* however, this assumption is totally wrong when there are one or more module
   * which are  */
  /* never used*/
  // use for each or standard lib to implement this instead of for loop
  for (list<VerilogModule *>::const_iterator itM = modules.begin();
       itM != modules.end(); ++itM) {
    for (list<Instantiation *>::iterator itI = (*itM)->instantiations->begin();
         itI != (*itM)->instantiations->end(); itI++) {
      for (list<VerilogModule *>::iterator t_itM = modules.begin();
           t_itM != modules.end(); ++t_itM) {
        if ((*t_itM)->name == (*itI)->type) {
          (*t_itM)->IsTopModule = false;
          break;
        }
      }
    }
  }

  int max_inst_num = 0;
  for (list<VerilogModule *>::const_iterator itM = modules.begin();
       itM != modules.end(); ++itM) {
    if ((*itM)->IsTopModule)
      if ((*itM)->instantiations->size() > max_inst_num)
        max_inst_num = (*itM)->instantiations->size();
  }

  int cnt = 0;
  for (list<VerilogModule *>::const_iterator itM = modules.begin();
       itM != modules.end(); ++itM) {
    if ((*itM)->IsTopModule) {
      cnt++;
      if ((*itM)->instantiations->size() == max_inst_num)
        _topModule = *itM;
      else
        (*itM)->IsTopModule = false;
    }
    if (cell_lib.find_module((*itM)->name))
      std::cout
          << "Warning : <module> : " << (*itM)->name
          << " is redefined! Please check your cell lib and verilog files."
          << std::endl;
  }
  if (cnt > 1) {
    std::cout << "There are more than one module which are never instantiated!"
              << std::endl;
  } else if (cnt == 1) {
    std::cout << "The Top Module is " << (*_topModule).name << std::endl;
  } else
    std::cout << "There is no one module which is not instantiated!"
              << std::endl;

  return 0;
}

int VerilogDesign::clarifyInstanceConnection() {
  for (list<VerilogModule *>::iterator it = modules.begin();
       it != modules.end(); it++) {
    (*it)->clarifyInstanceConnection(modules);
  }
  return 0;
}

void VerilogDesign::RegisterModules() {
  /*since right now the cell_lib is const*/
  /*we can't modify or add any new module into it*/
  // for (list<Module*> ::const_iterator itM = modules.begin();itM !=
  // modules.end();++itM)
  //{
  //	cell_lib.create_cell((*itM)->name, (*itM)->name);
  //	//need add port too but later
  // }
}
} // namespace VL2XML_PARSER