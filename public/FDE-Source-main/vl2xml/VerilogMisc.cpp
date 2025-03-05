/* (c) Copyright 2004-2005, Cadence Design Systems, Inc.  All rights reserved.

This file is part of the OA Gear distribution.  See the COPYING file in
the top level OA Gear directory for copyright and licensing information. */

/*
Author: Aaron P. Hurst <ahurst@eecs.berkeley.edu>

ChangeLog:
2006-12-25: ChangeLog started
*/

#include "VerilogMisc.h"

#define DELETE_UPON_DESTRUCTION 1

namespace VL2XML_PARSER {

class AlwaysBlock;
class Assignment;
class Bundle;
class Case;
class Declaration;
class Expression;
class Function;
class Instantiation;
class VerilogModule;
class Port;
class PortConnection;
class Primary;
class Statement;
class Trigger;

Function::~Function() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (declarations) {
    for (std::list<Declaration *>::iterator it = declarations->begin();
         it != declarations->end(); it++)
      delete (*it);
    delete declarations;
  }
  if (start)
    delete start;
  if (stop && start != stop)
    delete stop;
  if (action)
    delete action;
}

Declaration::~Declaration() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (start)
    delete start;
  if (stop && start != stop)
    delete stop;
  if (start2D)
    delete start2D;
  if (stop && start2D != stop2D)
    delete stop2D;
  if (value)
    delete value;
}

Trigger::~Trigger() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (net)
    delete net;
}

Assignment::~Assignment() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (lval)
    delete lval;
  if (value)
    delete value;
}

AlwaysBlock::~AlwaysBlock() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (action)
    delete action;
  if (triggers) {
    for (list<Trigger *>::iterator it = triggers->begin();
         it != triggers->end(); it++)
      delete (*it);
    delete triggers;
  }
}

Case::~Case() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (action)
    delete action;
  if (conditions) {
    for (list<Expression *>::iterator it = conditions->begin();
         it != conditions->end(); it++)
      delete (*it);
    delete conditions;
  }
}

Statement::~Statement() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (ifc.condition)
    delete ifc.condition;
  if (ifc.ifTrue)
    delete ifc.ifTrue;
  if (ifc.ifFalse)
    delete ifc.ifFalse;
  if (ifc.cases) {
    for (list<Case *>::iterator it = ifc.cases->begin(); it != ifc.cases->end();
         it++)
      delete (*it);
    delete ifc.cases;
  }
  if (assign.lval)
    delete assign.lval;
  if (assign.rval)
    delete assign.rval;
  if (begin_end.block) {
    for (list<Statement *>::iterator it = begin_end.block->begin();
         it != begin_end.block->end(); it++)
      delete (*it);
    delete begin_end.block;
  }
  if (begin_end.declarations) {
    for (list<Declaration *>::iterator it = begin_end.declarations->begin();
         it != begin_end.declarations->end(); it++)
      delete (*it);
    delete begin_end.declarations;
  }
}

Instantiation::~Instantiation() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (parameters) {
    for (list<Expression *>::iterator it = parameters->begin();
         it != parameters->end(); it++)
      delete (*it);
    delete parameters;
  }
  if (connections) {
    for (list<PortConnection *>::iterator it = connections->begin();
         it != connections->end(); it++)
      delete (*it);
    delete connections;
  }
}

PortConnection::~PortConnection() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (value)
    delete value;
}

Expression::~Expression() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (type == PRIMARY) {
    if (primary)
      delete primary;
    return;
  }
  if (type == BUNDLE) {
    if (primary)
      delete primary;
    return;
  }
  if (op1)
    delete op1;
  if (op2)
    delete op2;
  if (op3)
    delete op3;
}

Primary::~Primary() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  /* these destruction method seems to fail if it is a union because
         you can't tell this union really is list, range or number.
     there is no flag in it.so the problem is not in the destruction
         it's in the construction
   */
  if (range.start)
    delete range.start;
  if (range.stop && range.start != range.stop)
    delete range.stop;
  if (arguments) {
    for (list<Expression *>::iterator it = arguments->begin();
         it != arguments->end(); it++)
      delete (*it);
    delete arguments;
  }
}

Bundle::~Bundle() {
  if (!DELETE_UPON_DESTRUCTION)
    return;
  if (replication)
    delete replication;
  if (members) {
    for (list<Expression *>::iterator it = members->begin();
         it != members->end(); it++)
      delete (*it);
    delete members;
  }
}

} // namespace VL2XML_PARSER