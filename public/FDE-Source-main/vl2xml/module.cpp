#include "module.h"
#include "DebugHelper.hpp"
#include <boost/lexical_cast.hpp>
#include <stdio.h>
#include <utility>

using boost::lexical_cast;
using std::list;
using std::make_pair;
using std::map;
using std::string;
#define DELETE_UPON_DESTRUCTION 1

// #define _BUILD_CELLLIBRARY2_0
#define _PARSER_DEBUG

namespace VL2XML_PARSER {

/* get macro */

VerilogModule::VerilogModule()
    : IsTopModule(true), IsConverted(false), design(nullptr), ports(nullptr),
      assignments(nullptr), alwaysBlocks(nullptr), initialBlocks(nullptr),
      functions(nullptr), instantiations(nullptr) {}

VerilogModule::~VerilogModule() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  // delete ports
  if (ports) {
    for (list<VPort *>::iterator it = ports->begin(); it != ports->end(); it++)
      delete (*it);
    delete ports;
  }
  // delete assignments
  if (assignments) {
    for (list<Assignment *>::iterator it = assignments->begin();
         it != assignments->end(); it++)
      delete (*it);
    delete assignments;
  }
  // delete always blocks
  if (alwaysBlocks) {
    for (list<AlwaysBlock *>::iterator it = alwaysBlocks->begin();
         it != alwaysBlocks->end(); it++)
      delete (*it);
    delete alwaysBlocks;
  }
  // delete initial blocks
  if (initialBlocks) {
    for (list<Statement *>::iterator it = initialBlocks->begin();
         it != initialBlocks->end(); it++)
      delete (*it);
    delete initialBlocks;
  }
  // delete functions
  if (functions) {
    for (list<Function *>::iterator it = functions->begin();
         it != functions->end(); it++)
      delete (*it);
    delete functions;
  }
  // delete instantiations
  if (instantiations) {
    for (list<Instantiation *>::iterator it = instantiations->begin();
         it != instantiations->end(); it++)
      delete (*it);
    delete instantiations;
  }
  // delete declarations
  for (list<Declaration *>::iterator it = declarations.begin();
       it != declarations.end(); it++)
    delete (*it);
  // delete parameters
  for (list<Declaration *>::iterator it = parameters.begin();
       it != parameters.end(); it++)
    delete (*it);
  // delete parameter overrides
  if (parameterOverrides) {
    for (list<Declaration *>::iterator it = parameterOverrides->begin();
         it != parameterOverrides->end(); it++)
      delete (*it);
    delete parameterOverrides;
  }
  if (attributes) {
    for (list<Attribute *>::iterator it = attributes->begin();
         it != attributes->end(); ++it)
      delete (*it);
    delete attributes;
  }
}

/* The following functions are assumed to be used only in this file */
static string Dec2HexStr(int value, int width) {
  char buffer[40];
  snprintf(buffer, 40, "%X", value);
  string res(buffer);
  return res;
}
/* The following functions are member functions */
int VerilogModule::clarifyPorts() { return 0; }
int VerilogModule::clarifyAllNames() {
  /*Mainly check undefined module which is instantiated*/

  clarifyInstanceNames();

  clarifyNetNames();

  clarifyPortNames();

  /*need to print more info about the errors of the names*/
  return 0;
}

int VerilogModule::clarifyInstanceNames() { return 0; }

int VerilogModule::clarifyNetNames() { return 0; }

int VerilogModule::clarifyPortNames() { return 0; }

int VerilogModule::clarifyInstanceType(list<VerilogModule *> &modulelist,
                                       Library *cell_lib) {
  bool ModuleIsDefined = false;

  for (list<Instantiation *>::iterator itI = instantiations->begin();
       itI != instantiations->end(); itI++) {
    if (cell_lib->find_module((*itI)->type)) {
      continue;
    } else {
      for (list<VerilogModule *>::iterator itM = modulelist.begin();
           itM != modulelist.end(); ++itM) {
        if ((*itM)->name == (*itI)->type) {
          ModuleIsDefined = true;
          //(*itM)->IsTopModule = false;
          break;
        }
      }
      if (ModuleIsDefined) {
        ModuleIsDefined = false;
        continue;
      } else {
        /*need more care*/
        std::cout << "Can't find " << (*itI)->type
                  << " in neither CellLibrary and Module list" << std::endl;
        return 1;
      }
    }
  }
  return 0;
}

int VerilogModule::clarifyInstanceConnection(
    const list<VerilogModule *> &modulelist) {
  return 0;
}

int VerilogModule::ConvertModules(Module &cell, Library &cell_lib,
                                  list<VerilogModule *> &modules) {

  /*	clarifyNames is done by data structure I assume	*/
  /*	so no more check on names */

  /* ignore the glbl module which is for jtag */
  PUSH_STEP;
  int lineno;
  if (name == "glbl") {
    return 0;
  }

  if (ConvertParameters(cell, cell_lib, lineno)) {
    std::cout << "line  " << lineno
              << ":  An error happened in ConvertParameters!" << std::endl;
  }

  if (ConvertNets(cell, cell_lib, lineno)) {
    std::cout << "line  " << lineno << ":  An error happened in ConvertNets!"
              << std::endl;
  }

  if (ConvertInstances(cell, cell_lib, modules, lineno)) {
    std::cout << "line  " << lineno
              << ":  An error happened in ConvertInstances!" << std::endl;
  }

  if (ConvertAssignments(cell, cell_lib, lineno)) {
    std::cout << "line  " << lineno
              << ":  An error happened in ConvertAssignments" << std::endl;
  }

  if (ConvertAttributes(cell, cell_lib, lineno)) {
    std::cout << "line  " << lineno
              << ":  An error happened in ConvertAttributes" << std::endl;
  }
  if (ConvertParameterOverrides(cell, cell_lib, lineno)) {
    std::cout << "line  " << lineno
              << ":  An error happened in ConvertParameterOverrides"
              << std::endl;
  }

  /*if (ConvertPorts(cell, cell_lib))
  {
  std::cout << "An error happened in ConvertPorts!" << std::endl;
  }*/

  /*if (assignments) {
  for(list<Assignment *>::iterator it=assignments->begin();
  it!=assignments->end(); it++) {}
  }*/
  /*// convert always blocks
  if (alwaysBlocks) {
  for(list<AlwaysBlock *>::iterator it=alwaysBlocks->begin();
  it!=alwaysBlocks->end(); it++) {}
  }
  // convert initial blocks
  if (initialBlocks) {
  for(list<Statement *>::iterator it=initialBlocks->begin();
  it!=initialBlocks->end(); it++) {}

  }
  // convert functions
  if (functions) {
  for(list<Function *>::iterator it=functions->begin(); it!=functions->end();
  it++) {}
  }*/
  // convert parameter overrides
  /*if (parameterOverrides) {
          for(list<Declaration *>::iterator it=parameterOverrides->begin();
  it!=parameterOverrides->end(); it++) {}
  }*/
  this->IsConverted = true;
  POP_STEP;
  return 0;
}

int VerilogModule::ConvertParameters(Module &cell, Library &cell_lib,
                                     int &lineno) {
  PUSH_STEP;
  // convert parameters
  std::cout << "cell name:" << cell.name() << std::endl;
  for (list<Declaration *>::iterator it = parameters.begin();
       it != parameters.end(); it++) {
    std::cout << "value:" << (*it)->value << std::endl;
    std::cout << "prim:" << (*it)->value->primary << std::endl;
    Primary *prim = (*it)->value->primary;
    /* Used for the definition of width */
    if (prim->type == Primary::CONST) {
      _ParameterMap.insert(std::make_pair((*it)->name, prim->number.intValue));
      std::cout << "const" << std::endl;
    }
    /* Used for pass property */
    else if (prim->type == Primary::NET) {
      COS::Property<string> &temp =
          COS::create_property<string>(COS::MODULE, (*it)->name);
      cell.set_property<string>(temp, prim->name);
    } else {
      std::cout << "Parameter type not supported\n";
      std::cout << "Parameter name: " << (*it)->name << "\n";
      exit(1);
    }
    lineno = (*it)->lineno;
    // std::cout<< "parameter" << (*it)->lineno<<std::endl;
  }
  POP_STEP;
  return 0;
}

int VerilogModule::ConvertNets(Module &cell, Library &cell_lib, int &lineno) {
  // Create Nets Here
  // not support reg  [7:0] ram[7:0]
  PUSH_STEP;
  for (list<Declaration *>::iterator itD = declarations.begin();
       itD != declarations.end(); ++itD) {
    Expression *estart = (*itD)->start;
    Expression *estop = (*itD)->stop;
    // Primary *tempprim = new Primary((*itD)->name, estart, estop);
    list<string> namelist;
    int rstart = 0, rstop = 0;
    string decl_name = (*itD)->name;
    if (decl_name == "\\BRV/addr_b<0> ") {
      std::cout << "\n";
    }

    ParsePrimaryNet(decl_name, estart, estop, namelist, rstart, rstop);
    Declaration::Type t = (*itD)->type;

    // create port and correspondent net i.e input/output/inout
    if (t == Declaration::INPUT || t == Declaration::OUTPUT ||
        t == Declaration::INOUT) {
      if (!cell.ports().find(decl_name)) {
        /* Now treat the "input [n:n] ...;" declaration like a port group. So
         * when connect instance, find name with or without [n] will be good
         * enough */
        if ((rstart == -1 && rstop == -1)) {
          Port *p = cell.create_port(decl_name, COS::DirType(t));

#ifndef _BUILD_CELLLIBRARY2_0
          if (!instantiations->empty()) {
            p->mpin()->connect(cell.create_net(decl_name));
          }
#endif         /* _BUILD_CELLLIBRARY2_0 */
        } else // a multi bits port
        {
#ifndef _BUILD_CELLLIBRARY2_0
          Port *pg =
              cell.create_port(decl_name, rstart, rstop, COS::DirType(t));
          for (Module::pin_iter pi = pg->mpins().begin();
               pi != pg->mpins().end(); ++pi) {
            string n = pi->name();
            if (!instantiations->empty()) {
              pi->connect(cell.create_net(n));
            }
          }
#else
          for (list<string>::iterator itS = namelist.begin();
               itS != namelist.end(); ++itS) {
            if (!cell.find_port(*itS)) {
              Port *p = cell.create_port(*itS, COS::DirType(t));
            } else {
              std::cout << "The port : " << *itS << " is redefined!\n";
            }
          }
#endif /* _BUILD_CELLLIBRARY2_0 */
        }
      } else
        std::cout << "Repeated port declaration: " << decl_name << std::endl;
    } else // create net i.e wire/reg
    {
      for (list<string>::iterator itS = namelist.begin(); itS != namelist.end();
           ++itS) {
        // need to check whether the name is used to detect unintentional
        // mistake and wire/reg replaced declaration for port
        string net_name = *itS;
        if (!cell.nets().find(net_name)) {
          Net *n = cell.create_net(net_name);
        } else if (!cell.find_port(net_name)) {
          std::cout << "Warning : net <" << net_name << "> is redefined!\n";
          // exit(0);
        }
      }
    }
    lineno = (*itD)->lineno;
    // std::cout<< "NET" << (*itD)->lineno<<std::endl;
    // delete tempprim;//delete the pointer hoping it will not delete the
    // embedded pointer
  }
  return 0;
}
int VerilogModule::ConvertInstances(Module &cell, Library &cell_lib,
                                    list<VerilogModule *> &modules,
                                    int &lineno) {
  Module *cell_ptr;
  for (list<Instantiation *>::iterator it = instantiations->begin();
       it != instantiations->end(); it++) {
    /* find the prototype in the cell library to instantiate */
    /* and if didn't find */
    /* this check is also done in the clarifyInstanceType */
    if ((cell_ptr = cell_lib.find_module((*it)->type)) != nullptr)
      ; // std::cout << "Find " << (*it)->type << " in the CellLibrary!\n";
    else {
      bool b = false;
      for (list<VerilogModule *>::iterator mit = modules.begin();
           mit != modules.end(); ++mit) {
        if ((*it)->type == (*mit)->name && !((*mit)->IsConverted)) {
          std::cout << "Try to convert <module> : " << (*mit)->name
                    << " for <instance> : " << (*it)->name << " ..."
                    << std::endl;
          Module *newcell = cell_lib.create_module((*it)->type, "GENERIC");
          (*mit)->ConvertModules(*newcell, cell_lib, modules);
          std::cout << "Conversion of <module> :" << (*mit)->name
                    << " for <instance> : " << (*it)->name << " is done."
                    << std::endl;
          b = true;
          break;
        }
      }
      ASSERT(b, "line  " << (*it)->lineno
                         << ": Error in ConvertInstances. Module "
                         << (*it)->name << " is not defined!\n");
      // something need to be done with the module type and name
      cell_ptr = cell_lib.find_module((*it)->type);
    }

    Module *isntof = cell_ptr;
    string inst_name = (*it)->name;
    /* If the instance name is escaped, then delete the initial '\' and last '
     * '*/
    Instance *inst = cell.create_instance(inst_name, isntof);

    list<PortConnection *> *connections = (*it)->connections;

    Net::pin_iter pit = inst->pins().begin();
    // Deal with ordered name connections. Change them into ordered name format
    for (list<PortConnection *>::iterator itPC = connections->begin();
         itPC != connections->end(); ++itPC) {
      /// need to check how to get access to a port of an instance using index
      if ((*itPC)->position != -1) {
        (*itPC)->name = (*(pit + (*itPC)->position))->name();
      }
    }

    // Deal with named port connections: connect them. Note:support net
    // connection without exlicitly declare it if it's single bit.For multi
    // bits, report an error.
    for (list<PortConnection *>::iterator itPC = connections->begin();
         itPC != connections->end(); ++itPC) {
      string _portName = (*itPC)->name;
      Expression *expr = (*itPC)->value;

      list<string> namelist;

      // if expr is nullptr then the port is empty.now just ignore it.
      if (!expr)
        continue;
      else
        PrimaryOrBundle(expr, namelist, cell, cell_lib);

      ConnectInstance(cell, *inst, namelist, _portName);
    }

    // std::cout<< "instance" << (*it)->lineno<<std::endl;
    lineno = (*it)->lineno;
  }
  // std::cout << "line no is" << lineno << std::endl;
  return 0;
}

void VerilogModule::ConnectInstance(Module &cell, Instance &inst,
                                    list<string> &namelist, string portname) {
  Module *instof = inst.down_module();
  // If it belong to a port group, then treat it like it's a submodule
  // If not, treat it like it come from the cell lib
  const Port *pg = nullptr;
  for (const Port *port : instof->ports()) {
    if (port->is_vector())
      if (port->name() == portname) {
        pg = port;
        break;
      }
  }

  if (pg) {
    int msb = pg->msb(), lsb = pg->lsb();
    list<string>::iterator name_it = namelist.begin();
    if (static_cast<size_t>(pg->width()) != namelist.size()) {
      ; // Need to add warning here
    }
    for (const Pin *port : pg->mpins()) {
      string port_name = port->name();
      inst.find_pin(port->name())->connect(cell.find_net(*name_it));
      ++name_it;
    }
  } else {
    /* if namelist contains more than one wire, then the instance port is a
     * multi-bits port. Now not */
    /* only need to scalarize the net buy also scalarize the port */
    if (namelist.size() > 1) {
      int index = namelist.size() - 1;
      for (list<string>::iterator it = namelist.begin(); it != namelist.end();
           ++it, --index) {
        if (*it != "HIGHZ!") {
          string tp = portname + "[" + lexical_cast<string>(index) +
                      "]"; // remember scale the port from high to low	because
                           // the namelist contain nets from high to low
          inst.find_pin(tp)->connect(cell.find_net(*it)); // multi bits port
        } else
          ; // just do nothing for high bit of RAM
      }
    } else { // just single bit net in the namelist
      string n = *(namelist.begin());
      if (n != "HIGHZ!") {
        if (inst.find_pin(portname)) {
          if (cell.find_net(n))
            inst.find_pin(portname)->connect(cell.nets().find(n));
          else {
            std::cout << "Warning : <net> : " << n
                      << " is used without explicitly declaration!"
                      << std::endl;
            inst.find_pin(portname)->connect(cell.create_net(n));
          }
        } else if (inst.find_pin(portname + "[" + lexical_cast<string>(0) +
                                 "]")) {
          if (cell.find_net(n))
            inst.find_pin(portname + "[" + lexical_cast<string>(0) + "]")
                ->connect(cell.find_net(n));
          else {
            std::cout << "Warning : <net> : " << n
                      << " is used without explicitly declaration!"
                      << std::endl;
            inst.find_pin(portname + "[" + lexical_cast<string>(0) + "]")
                ->connect(cell.create_net(n));
          }
        } else {
          // can't match any possible port name in cell library and other sub
          // modules
          std::cout << "Parser can't find <port>: " << portname << " or "
                    << portname + "[" + lexical_cast<string>(0) + "]"
                    << " in <instance>: " << inst.name() << std::endl;
          std::cout << " Please check if you use the right cell library or any "
                       "spelling mistakes."
                    << std::endl;
          exit(1);
        }
        // single bit net but have you considered dangling ports??
      } else {
        // just do nothing for high bit of RAM
      }
    }
  }
}

int VerilogModule::ConvertAssignments(Module &cell, Library &cell_lib,
                                      int &lineno) {
  /* actually after the assignments, the two nets on the both sides of '=' is
   * all the same	*/
  /* so now delete one which is type 'wire' and keep the port one
   */
  /* if both of the net is type 'wrie',then delete the left one
   */

  /* to handle the assignments ,first divide the nets involved into separate
   * equivalent groups */
  map<string, int> eg;
  map<string, int>::iterator iter;
  int group_no = 0;
  for (list<Assignment *>::iterator itA = assignments->begin();
       itA != assignments->end(); ++itA) {
    /* first take the real net out and convert it if necessary (i.e ain[1])*/
    Expression *lval = (*itA)->lval;
    Expression *value = (*itA)->value;
    list<string> lnetname, rnetname;
    PrimaryOrBundle(lval, lnetname, cell, cell_lib);
    PrimaryOrBundle(value, rnetname, cell, cell_lib);
    list<string>::iterator lit = lnetname.begin(), rit = rnetname.begin();
    if (lnetname.size() != rnetname.size()) {
      std::cout
          << "line  " << (*itA)->lineno
          << ":  Error in assignment. The width on both sides are not the same!"
          << std::endl;
      exit(1);
    }
    for (; lit != lnetname.end(); ++lit, ++rit) {
      if (eg.find(*lit) == eg.end() && eg.find(*rit) == eg.end()) {
        eg.insert(make_pair(*lit, group_no));
        eg.insert(make_pair(*rit, group_no));
        ++group_no;
      } else if (eg.find(*lit) != eg.end() && eg.find(*rit) == eg.end()) {
        iter = eg.find(*lit);
        eg.insert(make_pair(*rit, iter->second));
      } else if (eg.find(*lit) == eg.end() && eg.find(*rit) != eg.end()) {
        iter = eg.find(*rit);
        eg.insert(make_pair(*lit, iter->second));
      } else if (eg.find(*lit) != eg.end() && eg.find(*rit) != eg.end()) {
        std::cout << "line  " << (*itA)->lineno
                  << "Error!!Redundant assignments appear!" << std::endl;
      }
    }
    // for (;lit != lnetname.end(); ++lit, ++rit)
    //{
    //	if (cell.ports().find(*lit) && !cell.ports().find(*rit))
    //	{
    //		Module::net_iter li = cell.nets().find_it(*lit);
    //		Module::net_iter ri = cell.nets().find_it(*rit);
    //		for (int n = 0;n < ri->num_pins();++n)
    //		{
    //			Net::pin_iter pit = ri->pins().begin();
    //			//string pname = pit->name();
    //			//Pin &pin = *pit;
    //			pit->rehook(*li);
    //		}
    //		cell.remove_net(ri);
    //	}
    // }
    lineno = (*itA)->lineno;
    // std::cout <<"assignments"<< (*itA)->lineno << std::endl;
  }
  /* secondly, merge the equivalent group into only one net */
  MergeNets(&eg, cell, group_no);

  return 0;
}

int VerilogModule::ConvertParameterOverrides(Module &cell, Library &cell_lib,
                                             int &lineno) {
  for (list<Declaration *>::iterator it = parameterOverrides->begin();
       it != parameterOverrides->end(); it++) {
    Expression *value = (*it)->value;
    Primary *p = value->primary;
    string name0 = (*it)->name;
    string s1 = name0.substr(name0.find_last_of('.'));
    string s2 = s1.substr(s1.find_first_not_of('.'));
    string s3 =
        name0.substr(0, name0.find_last_of('.')); // only support ' ' i.e space.
    if (s3[0] == '\\')
      ;
    else
      s3 = s3.substr(0, s3.find_first_of(' '));

    COS::Property<string> &temp =
        COS::create_property<string>(COS::INSTANCE, s2);
    if (p->type == Primary::CONST) {
      Instance *inst = cell.instances().find(s3);
      if (inst) {
        if (p->number.bitWidth >= 30) {
          inst->set_property(temp, p->strValue);
        } else {
          string v = Dec2HexStr(p->number.intValue, p->number.bitWidth);
          inst->set_property(temp, v);
        }
      } else {
        std::cout << "line  " << (*it)->lineno << " Error : <instance> : " << s3
                  << " doesn't exit! Please check your verilog files."
                  << std::endl;
        exit(1);
      }
    } else if (p->type == Primary::NET) {
      Instance *inst = cell.instances().find(s3);
      if (inst)
        inst->set_property(temp, p->name);
      else {
        std::cout << "line  " << (*it)->lineno << " Error : <instance> : " << s3
                  << " doesn't exit! Please check your verilog files."
                  << std::endl;
        exit(1);
      }
    }
    // std::cout <<"assignments"<< (*it)->lineno << std::endl;
    lineno = (*it)->lineno;
  }
  return 0;
}
int VerilogModule::ConvertAttributes(Module &cell, Library &cell_lib,
                                     int &lineno) {
  COS::Property<Point> &temp1 =
      COS::create_property<Point>(COS::INSTANCE, "rloc");
  COS::Property<Point> &temp2 =
      COS::create_property<Point>(COS::INSTANCE, "h_set");
  COS::Property<Point> &temp3 =
      COS::create_property<Point>(COS::INSTANCE, "mode");

  for (list<Attribute *>::iterator it = attributes->begin();
       it != attributes->end(); ++it) {
    // cell.instances().find((*it)->owner)->set_property<string>("cfgMode",
    // (*it)->attr, PERSIST);
    if ((*it)->type == "rloc") {
      cell.find_instance((*it)->owner)
          ->set_property(temp1, lexical_cast<Point>((*it)->attr));
    } else if ((*it)->type == "h_set") {
      cell.find_instance((*it)->owner)
          ->set_property(temp2, lexical_cast<Point>((*it)->attr));
    } else if ((*it)->type == "mode") {
      cell.find_instance((*it)->owner)
          ->set_property(temp3, lexical_cast<Point>((*it)->attr));
    }
    lineno = (*it)->lineno;
    // std::cout<<"attributes "<<(*it)->lineno<<std::endl;
  }
  return 0;
}

int VerilogModule::ConvertPorts(Module &cell, Library &cell_lib, int &lineno) {

  /* a module's port can be derived from the declarations */
  for (list<VPort *>::iterator it = ports->begin(); it != ports->end(); it++) {
    // cell.create_port((*it).externalName, Pin::unknown);
  }
  return 0;
}

void VerilogModule::PrimaryOrBundle(Expression *expr, list<string> &namelist,
                                    Module &cell, COS::Library &cell_lib) {
  if (expr->type == Expression::PRIMARY) {
    Primary *prim = (expr->primary);
    if (prim->type == Primary::NET) {
      ParsePrimaryNet(prim, namelist);
    } else if (prim->type == Primary::CONST) { // e.g 1'b0
      /* first, create a LOGIC0 or LOGIC1 instance if it is not created yet */
      /* second, create a net and hookup the LOGIC to the net */
      /* third, push the net's name in the namelist */

      int bw = prim->number.bitWidth;
      int v = prim->number.intValue;
      int xm = prim->number.xMask;
      int zm = prim->number.zMask;
      if (zm) {
        // now if bw != zm then use the bw.
        /*if (zm != bw)
        {
                std::cout <<
        }*/
        for (int i = 0; i < zm; ++i) {
          namelist.push_back("HIGHZ!");
        }
      } else if (xm) {
        for (int i = 0; i < xm; ++i) {
          namelist.push_back("XXXXX!");
        }
      } else if (v != 0) { // this node connect to vcc
        if (!cell.instances().find("logic1_VCC")) {
          Module *isntof = cell_lib.find_module("LOGIC_1");
          Instance *inst = cell.create_instance("logic1_VCC", isntof);
          Net *net = cell.create_net("logic1_rails");
          inst->find_pin("LOGIC_1_PIN")
              ->connect(cell.nets().find("logic1_rails"));
        }
      } else { // this node connect to gnd
        if (!cell.instances().find("logic0_GND")) {
          Module *isntof = cell_lib.find_module("LOGIC_0");
          Instance *inst = cell.create_instance("logic0_GND", isntof);
          Net *net = cell.create_net("logic0_rails");
          inst->find_pin("LOGIC_0_PIN")
              ->connect(cell.nets().find("logic0_rails"));
        }
      }
      if ((!zm) && (!xm)) {
        for (int i = 0; i < bw; ++i) {
          if (v != 0) {
            namelist.push_back("logic1_rails");
          } else {
            namelist.push_back("logic0_rails");
          }
        }
      }
    }
  } else if (expr->type == Expression::BUNDLE) {
    Bundle *bundle = (expr->bundle);
    ParseBundleNet(bundle, namelist, cell, cell_lib);
  }
}

void VerilogModule::ParsePrimaryNet(string name, Expression *start,
                                    Expression *stop, list<string> &namelist,
                                    int &rstart, int &rstop) {
  PUSH_STEP;
  /* this function's function is the same as the function below*/
  /* while take different parameters because this is used in declaration*/
  /* while the one below handle the instance port connection	*/
  string netname;
  // standard identifier the start will be nullptr the same time
  if (start == nullptr || stop == nullptr) {
    namelist.push_back(name);
    rstart = -1;
    rstop = -1;
  }
  // this condition is never met in net declaration but quite common in port
  // connections
  else if (start == stop) // single select net e.g cout[1] or 'wire [1:1] in;'
  {
    Primary *p = start->primary;
    if (p->type != Primary::CONST)
      CheckMapItem(_ParameterMap, "Parameter", p->name);
    int i = (p->type == Primary::CONST) ? p->number.intValue
                                        : _ParameterMap[p->name];
    netname = name + "[" + lexical_cast<string>(i) + "]";
    /* because some identifier generated by DC will be the same with the scalar
     * net's name		*/
    /* generated by parser. so the parser will leave the scalar net's name
     * untouched			*/
    /* and this will cause the variable's name unconformed with Verilog standard
     * and may cause	*/
    /* problems in the modelsim simulation.The following is the standard name
     * "\\" + " "		*/
    /* netname = "\\" + name + "[" + lexical_cast<string>(i)  + "] ";
     */
    /* all the function gererating names will be affected
     */
    namelist.push_back(netname);
  } else if (start != stop) // use the Expression(Primary*) constructor
  {
    Primary *pstart = start->primary;
    Primary *pstop = stop->primary;
    int vstart, vstop;
    if (pstart->type != Primary::CONST)
      CheckMapItem(_ParameterMap, "Parameter", pstart->name);
    if (pstop->type != Primary::CONST)
      CheckMapItem(_ParameterMap, "Parameter", pstop->name);
    vstart = (pstart->type == Primary::CONST)
                 ? pstart->number.intValue
                 : _ParameterMap[pstart->name]; // need parse the parameter.
                                                // same below
    vstop = (pstop->type == Primary::CONST) ? pstop->number.intValue
                                            : _ParameterMap[pstop->name];

    /********************scalarize net here***************************/
    if (vstart == vstop) {
      netname = name + "[" + lexical_cast<string>(vstart) +
                "]"; // ###if a net declared like "wire [0:0] n1;" then the n1
                     // should be n1[0] in xml not n1###
      namelist.push_back(netname);
    } else if (vstop != vstart) {
      // It seems modelsim only support the "\multi [0]" format. So treat the
      // escaped and non-escaped equally.Just comment it out.

      for (int i = (vstart > vstop ? vstart : vstop);
           i >= (vstart > vstop ? vstop : vstart); i--) {
        netname = name + "[" + lexical_cast<string>(i) + "]";
        namelist.push_back(netname);
      } // assume that identifiers with "\" can only be single bit.and
        // multi-bitwidth can't follow escaped identifier
      // need further research because the lex file seems support the escaped
      // identifier with multi-bits}
    }

    // change the name order
    rstart = vstart > vstop ? vstart : vstop;
    rstop = vstart > vstop ? vstop : vstop;
  }
  POP_STEP;
}

void VerilogModule::ParsePrimaryNet(Primary *prim, list<string> &namelist) {
  PUSH_STEP;
  string netname;
  // standard identifier the start and stop will be nullptr the same time but
  // may still by multi bits like ADDR if so you need to consider that condition
  if (prim->range.start == nullptr || prim->range.stop == nullptr) {
    // use the STL algorithm find_if may perform better but i don't know it yet
    // ^_^
    bool singlebit = true;
    for (list<Declaration *>::iterator itD = declarations.begin();
         itD != declarations.end(); ++itD) {
      if ((*itD)->name == prim->name) {
        singlebit = false;
        Expression *estart = (*itD)->start;
        Expression *estop = (*itD)->stop;
        int no_use_a, no_use_b;
        ParsePrimaryNet((*itD)->name, estart, estop, namelist, no_use_a,
                        no_use_b);
        break;
      }
    }
    if (singlebit) {
      namelist.push_back(prim->name);
    }
  }
  // this condition is never met in net declaration but quite common in port
  // connections
  else if (prim->range.start ==
           prim->range
               .stop) // single select net e.g cout[1] or 'wire [1:1] in;'
  {
    Primary *p = prim->range.start->primary;
    if (p->type != Primary::CONST)
      CheckMapItem(_ParameterMap, "Parameter", p->name);
    int i = (p->type == Primary::CONST) ? p->number.intValue
                                        : _ParameterMap[p->name];
    netname = prim->name + "[" + lexical_cast<string>(i) + "]";
    namelist.push_back(netname);
  } else if (prim->range.start !=
             prim->range.stop) // use the Expression(Primary*) constructor
  {
    Primary *pstart = prim->range.start->primary;
    Primary *pstop = prim->range.stop->primary;
    int vstart, vstop;
    if (pstart->type != Primary::CONST)
      CheckMapItem(_ParameterMap, "Parameter", pstart->name);
    if (pstop->type != Primary::CONST)
      CheckMapItem(_ParameterMap, "Parameter", pstop->name);
    vstart = (pstart->type == Primary::CONST)
                 ? pstart->number.intValue
                 : _ParameterMap[pstart->name]; // need parse the parameter.
                                                // same below
    vstop = (pstop->type == Primary::CONST) ? pstop->number.intValue
                                            : _ParameterMap[pstop->name];

    /********************scalarize net here***************************/
    if (vstart == vstop) {
      netname = prim->name + "[" + lexical_cast<string>(vstart) + "]";
      namelist.push_back(netname);
    } else if (vstop != vstart) {
      for (int i = (vstart > vstop ? vstart : vstop);
           i >= (vstart > vstop ? vstop : vstart); i--) {
        netname = prim->name + "[" + lexical_cast<string>(i) + "]";
        namelist.push_back(netname);
      } // assume that identifiers with "\" can only be single bit.and
        // multi-bitwidth can't follow escaped identifier
    }   // need further research because the lex file seems support the escaped
        // identifier with multi-bits
  }
  POP_STEP;
}
void VerilogModule::ParseBundleNet(Bundle *bundle, list<string> &namelist,
                                   Module &cell, Library &cell_lib) {
  PUSH_STEP;
  string netname;
  list<Expression *> *exprList = bundle->members;
  for (list<Expression *>::iterator it = exprList->begin();
       it != exprList->end(); ++it) {
    if ((*it)->type == Expression::PRIMARY) {
      Primary *prim = (*it)->primary;
      if (prim->type == Primary::NET) {
        ParsePrimaryNet(prim, namelist);
      } else if (prim->type == Primary::CONST) {
        // Logic0&1 in a bundle
        // ParsePrimaryNet(prim, namelist);
        int bw = prim->number.bitWidth;
        int v = prim->number.intValue;
        int xm = prim->number.xMask;
        int zm = prim->number.zMask;

        if (zm) {
          // now if bw != zm then use the bw.
          /*if (zm != bw)
          {
          std::cout <<
          }*/
          for (int i = 0; i < zm; ++i) {
            namelist.push_back("HIGHZ!");
          }
        } else if (xm) {
          for (int i = 0; i < xm; ++i) {
            namelist.push_back("XXXXX!");
          }
        } else if (v != 0) {
          if (!cell.instances().find("logic1_VCC")) {
            Module *isntof = cell_lib.find_module("LOGIC_1");
            Instance *inst = cell.create_instance("logic1_VCC", isntof);
            Net *net = cell.create_net("logic1_rails");
            inst->find_pin("LOGIC_1_PIN")
                ->connect(cell.nets().find("logic1_rails"));
          }
        } else {
          if (!cell.instances().find("logic0_GND")) {
            Module *isntof = cell_lib.find_module("LOGIC_0");
            Instance *inst = cell.create_instance("logic0_GND", isntof);
            Net *net = cell.create_net("logic0_rails");
            inst->find_pin("LOGIC_0_PIN")
                ->connect(cell.nets().find("logic0_rails"));
            Pin &pina = *(inst->find_pin("LOGIC_0_PIN"));
            std::cout << "Hello";
          }
        }
        if ((!zm) && (!xm)) {
          for (int i = 0; i < bw; ++i) {
            if (v != 0) {
              namelist.push_back("logic1_rails");
            } else {
              namelist.push_back("logic0_rails");
            }
          }
        }
      }
    } else if ((*it)->type == Expression::BUNDLE) {
      Bundle *bun = (*it)->bundle;
      list<string> subNameList;
      ParseBundleNet(bun, subNameList, cell, cell_lib);
      namelist.splice(namelist.end(), subNameList);
    }
  }
  POP_STEP;
}

void VerilogModule::MergeNets(map<string, int> *eg, Module &cell, int gn) {
  PUSH_STEP;
  int i = 0;
  for (; i != gn; ++i) {
    string tmp = "";
    Net *n = cell.create_net(tmp);
    for (map<string, int>::iterator it = eg->begin(); it != eg->end(); ++it) {
      if ((*it).second == i) {
        Net *ni = cell.find_net((*it).first);
        CheckPointer(ni, "Net", (*it).first);
        Net &n1 = *ni;
        if (tmp == "") {
          if (cell.find_port(ni->name())) {
            tmp = ni->name();
          }
        }
        int pin_num = ni->num_pins();
        for (int j = 0; j < pin_num; ++j) {
          Net::pin_iter pi = ni->pins().begin();
          Pin *p_ptr = *pi;
          pi->reconnect(n);
        }
        Net *n2 = cell.find_net((*it).first);
        CheckPointer(n2, "Net", (*it).first);
        cell.remove_net(ni);
      }
    }
    n->rename(tmp);
  }
  POP_STEP;
}
} // namespace VL2XML_PARSER