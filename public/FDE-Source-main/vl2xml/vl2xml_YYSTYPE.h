/*
*	File Description:

*	Author:
*	Date:
*
*/

#ifndef VL2XML_YYSTYPE_H
#define VL2XML_YYSTYPE_H

#include "VerilogMisc.h"
#include "module.h"
#include <list>
#include <map>
#include <string>

using std::list;
using std::map;
using std::string;

using namespace VL2XML_PARSER;

// class AlwaysBlock;
// class Assignment;
// class Bundle;
// class Case;
// class Declaration;
// class Expression;
// class Function;
// class Instantiation;
// class VPort;
// class PortConnection;
// class Primary;
// class Statement;
// class Trigger;
// class Module;

typedef union YYSTYPE {
  struct {
    bool negative; // sign flag
    int bits;      // bit length
    int radix;
    unsigned int value; // integer value
    unsigned int zmask; // mask of bits that were 'z'
    unsigned int xmask; // mask of bits that were 'x'
  } number;
  struct {
    char *s; // string
    int Line_no;
  } str;

  // char*                                   str;			//
  // string
  char *strValue; // string version of primary value
  // int                                     Line_no;
  Expression *expression;
  list<Expression *> *expressions;
  Primary *primary;
  list<Primary *> *primaries;
  Bundle *bundle;
  struct {
    struct {
      bool negative; // sign flag
      int bits;      // bit length
      int radix;
      unsigned int value; // integer value
      unsigned int zmask; // mask of bits that were 'z'
      unsigned int xmask; // mask of bits that were 'x'
    } number;
    char *strValue; // string
  } Num;
  VL2XML_PARSER::VerilogModule *module;
  list<VPort *> *ports;
  list<PortConnection *> *portConnections;
  Assignment *assignment;
  Declaration *declaration; /*no use so far*/
  PortConnection *portConnection;
  Instantiation *instantiation;

  Statement *statement;
  list<Statement *> *statements;
  list<Case *> *cases;

  Trigger *trigger;
  list<Trigger *> *triggers;

  Attribute *attribute;

  struct {
    list<Assignment *> *assignments;
    list<AlwaysBlock *> *alwaysBlocks;
    list<Statement *> *initialBlocks;
    list<Instantiation *> *instantiations;
    list<Declaration *> *declarations;
    list<Function *> *functions;
    list<Declaration *> *parameterOverrides;
    list<Attribute *> *attributes;
  } decls;
  YYSTYPE() = default;
} YYSTYPE;

#define YYSTYPE_IS_DECLARED 1
#define YYSTYPE_IS_TRIVIAL 1

#endif