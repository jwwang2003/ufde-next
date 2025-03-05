#ifndef VERILOGMISC_H
#define VERILOGMISC_H

#include <list>
#include <string>
#include <vector>

namespace VL2XML_PARSER {

class AlwaysBlock;
class Assignment;
class Bundle;
class Case;
class Declaration;
class Expression;
class Function;
class Instantiation;
class VPort;
class PortConnection;
class Primary;
class Statement;
class Trigger;

using std::list;
using std::string;
// *****************************************************************************
// Port
//
/// \brief A port definition (from inside a portlist).
//
// *****************************************************************************
class VPort {
public:
  string externalName;
  int position;
  string internalName;
  int lineno = 0; // line number
  // Destructor
  ~VPort() {}
};

// *****************************************************************************
// Statement
//
/// \brief A procedural statement.
///
/// The statement can be one of the following types:
/// \li case
/// \li if / if-else
/// \li blocking assignment
/// \li non-blocking assignment
/// \li begin-end block
//
// *****************************************************************************

class Statement {
public:
  typedef enum {
    NOP,
    BLOCK,
    IF,
    CASE,
    CASEX,
    CASEZ,
    BLOCKING_ASSIGNMENT,
    NONBLOCKING_ASSIGNMENT
  } Type;
  Type type;
  int lineno; // line number
  // for if/case statements...
  struct {
    Expression *condition;
    Statement *ifTrue;
    Statement *ifFalse;

    list<Case *> *cases;
  } ifc;

  // for assign statements...
  struct {
    Expression *lval;
    Expression *rval;
  } assign;

  // for begin/end statements...
  struct {
    list<Statement *> *block;
    list<Declaration *> *declarations;
  } begin_end;

  string name;

  Statement() {
    lineno = 0;
    type = NOP;
    ifc.condition = nullptr;
    ifc.ifTrue = ifc.ifFalse = nullptr;
    ifc.cases = nullptr;
    assign.lval = assign.rval = nullptr;
    begin_end.block = nullptr;
    begin_end.declarations = nullptr;
  }
  // Destructor
  ~Statement();
};

// **************************************************************************
// PortConnection
//
/// \brief A port connection inside of a module instantiation.
//
// *****************************************************************************

class PortConnection {
public:
  string name;
  int position;
  Expression *value;
  int lineno; // line number
  PortConnection() {
    position = -1;
    value = nullptr;
    lineno = 0;
  }
  PortConnection(const string &n, int p, Expression *v) {
    name = n;
    position = p;
    value = v;
    lineno = 0;
  }

  // Destructor
  ~PortConnection();
};

// *****************************************************************************
// Instantiation
//
/// \brief A module instantiation.
//
// *****************************************************************************

class Instantiation {
public:
  string name;
  string type;
  typedef enum {
    ISNT_PRIMITIVE,
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    XNOR,
    NOT,
    BUF
  } PrimitiveType;
  PrimitiveType primitive;

  list<Expression *> *parameters;
  list<PortConnection *> *connections;
  int lineno; // line number
  Instantiation() {
    parameters = nullptr;
    connections = nullptr;
    lineno = 0;
  }
  // Destructor
  ~Instantiation();
};

// *****************************************************************************
// Function
//
/// \brief A function definition
//
// *****************************************************************************

class Function {
public:
  string name;

  Expression *start, *stop;

  list<Declaration *> *declarations;
  Statement *action;
  int lineno; // line number
  Function() {
    start = stop = nullptr;
    declarations = nullptr;
    action = nullptr;
    lineno = 0;
  }
  // Destructor
  ~Function();
};

// *****************************************************************************
// Primary
//
/// \brief An expression primary.
///
/// The primary can either be:
/// \li a constant value
/// \li a parameter
/// \li a net or register
/// \li a function call
//
// *****************************************************************************

class Primary {
public:
  typedef enum { UNKNOWN, NET, CONST, FUNCTION_CALL } Type;
  Type type;
  int lineno; // line number
  string name;

  /* break the union into separate parts
  assume that there is enough memory
  and won't give rise to any bug */

  struct {
    bool negative;
    unsigned int intValue;
    int bitWidth;
    unsigned int xMask, zMask;
  } number;

  string strValue;

  struct {
    Expression *start;
    Expression *stop;
  } range;

  list<Expression *> *arguments;

  Primary(const string &n) {
    // scalar net primary (of name n)
    name = n;
    range.start = range.stop = nullptr;
    type = NET;
    arguments = nullptr;
    lineno = 0;
  }
  Primary(const string &str, int bitwidth) {
    strValue = str;
    number.bitWidth = bitwidth;
    type = CONST;
    range.start = range.stop = nullptr;
    arguments = nullptr;
    number.negative = false;
    lineno = 0;
  }
  Primary(unsigned int v, unsigned int w) {
    // constant primary (of value v and bitwidth w)
    range.start = range.stop = nullptr;
    number.intValue = v;
    number.bitWidth = w;
    type = CONST;
    number.negative = false;
    arguments = nullptr;
    lineno = 0;
  }
  Primary(const string &n, Expression *i) {
    // single-bit net primary (of name n and index i)
    name = n;
    range.start = i = range.stop = i;
    type = NET;
    arguments = nullptr;
    lineno = 0;
  }
  Primary(const string &n, Expression *f, Expression *l) {
    // multi-bit net primary (of name n and indices f to l)
    name = n;
    range.start = f;
    range.stop = l;
    type = NET;
    arguments = nullptr;
    lineno = 0;
  }
  Primary(const string &n, list<Expression *> *args) {
    // functional call primary (of name n with arguments)
    name = n;
    arguments = args;
    type = FUNCTION_CALL;
    range.start = range.stop = nullptr;
    lineno = 0;
  }

  // Destructor
  ~Primary();
};

// *****************************************************************************
// Expression
//
/// \brief An expression.
///
/// The expression is broken into single operations and built recursively.
///
/// Each decomposed expression may be involve one operator from the following
/// classes: \li arithmetic: +, -, *, /, % \li bit-wise: ~, &, |, ^, ~&, ~|, ~^,
/// ^~ \li logical: !, &&, || \li shift: <<, >> \li conditional:  ?/: \li
/// reduction: &, |, ^, ~&, ~|, ~^, ^~ \li equality: ==, !=, ===, !== \li
/// comparison: <, >, <=, >= \li bundle \li primary
//
// *****************************************************************************

class Expression {

public:
  typedef enum {
    UNKNOWN,
    PRIMARY,
    BUNDLE,
    BITWISE_AND,
    BITWISE_NAND,
    BITWISE_OR,
    BITWISE_NOR,
    BITWISE_XOR,
    BITWISE_XNOR,
    BITWISE_NOT,
    LOGICAL_AND,
    LOGICAL_NOT,
    LOGICAL_OR,
    REDUCTION_AND,
    REDUCTION_OR,
    REDUCTION_XOR,
    REDUCTION_NAND,
    REDUCTION_NOR,
    REDUCTION_XNOR,
    LESS_THAN,
    LESS_THAN_EQUAL,
    GREATER_THAN,
    GREATER_THAN_EQUAL,
    EQUAL,
    NOTEQUAL,
    IF_ELSE,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULO,
    NEGATE
  } Operator;
  Operator type;

  union {
    Expression *op1;
    Primary *primary;
    Bundle *bundle;
  };
  int lineno; // line number
  Expression *op2, *op3;
  Expression() {
    op1 = op2 = op3 = nullptr;
    type = UNKNOWN;
    lineno = 0;
  }

  /* this constructor is used to construct the expression for all the primary
   * nets */
  /* a primary net is a set of net with different index while sharing same
   * identifier */
  Expression(Primary *p) {
    primary = p;
    type = PRIMARY;
    lineno = 0;
  }

  /* this constructor is used to construct the expression for a set of nets
   * which is concatenated */
  /* the function of this set of nets is the same with the primary i.e the real
   * wires, regs or ports */
  Expression(Bundle *b) {
    bundle = b;
    type = BUNDLE;
    lineno = 0;
  }

  Expression(Expression *e1) {
    op1 = e1;
    op2 = op3 = nullptr;
    type = UNKNOWN;
    lineno = 0;
  }
  Expression(Expression *e1, Expression *e2) {
    op1 = e1;
    op2 = e2;
    op3 = nullptr;
    type = UNKNOWN;
    lineno = 0;
  }
  Expression(Expression *e1, Expression *e2, Expression *e3) {
    op1 = e1;
    op2 = e2;
    op3 = e3;
    type = UNKNOWN;
    lineno = 0;
  }

  // Destructor
  ~Expression();
};

// *****************************************************************************
// Declaration
//
/// \brief A wire, port, reg, or data type declaration.
///
/// Five declaration types are currently supported:
/// \li wire
/// \li input, output, inout
/// \li reg
//
// *****************************************************************************

class Declaration {
public:
  string name;
  typedef enum {
    UNKNOWN = 0,
    INPUT,
    OUTPUT,
    INOUT,
    WIRE,
    REG,
    PARAMETER,
    SUPPLY0,
    SUPPLY1,
    TRI,
    WIREAND, // not commonly used
    WIREOR,
    TRI0,
    TRI1,
    TRIAND,
    TRIOR
  } Type;
  Type type;

  Expression *start, *stop;
  Expression *start2D, *stop2D;

  Expression *value;
  int lineno; // line number
  Declaration() {
    start = stop = start2D = stop2D = value = nullptr;
    lineno = 0;
  }
  // Destructor
  ~Declaration();
};

// *****************************************************************************
// Trigger
//
/// \brief A procedural trigger.
///
/// A trigger may be one of three types:
/// \li posedge
/// \li negedge
/// \li both edges/any change
//
// *****************************************************************************

class Trigger {
public:
  typedef enum { UNKNOWN, BOTH, POSEDGE, NEGEDGE } Type;
  Type type;
  Expression *net;

  Trigger() { net = nullptr; }
  // Destructor
  ~Trigger();
};

// *****************************************************************************
// AlwaysBlock
//
/// \brief An always block.
//
// *****************************************************************************

class AlwaysBlock {
public:
  list<Trigger *> *triggers;
  Statement *action;

  AlwaysBlock() {
    triggers = nullptr;
    action = nullptr;
  }
  // Destructor
  ~AlwaysBlock();
};

// *****************************************************************************
// Case
//
/// \brief A single case inside of a case statement.
//
// *****************************************************************************
class Case {
public:
  list<Expression *> *conditions;
  Statement *action;
  bool isDefault;

  Case() {
    conditions = nullptr;
    action = nullptr;
  }
  // Destructor
  ~Case();
};

// *****************************************************************************
// Assignment
//
/// \brief A continuous assignment.
//
// *****************************************************************************

class Assignment {
public:
  Expression *lval;
  Expression *value;

  Assignment() {
    lval = value = nullptr;
    lineno = 0;
  }
  // Destructor
  ~Assignment();
  int lineno; // line number
};

class Bundle {
public:
  list<Expression *> *members;
  Expression *replication;

  Bundle(list<Expression *> *m) {
    members = m;
    replication = nullptr;
    lineno = 0;
  }
  Bundle(list<Expression *> *m, class Expression *r) {
    members = m;
    replication = r;
    lineno = 0;
  }
  // Destructor
  ~Bundle();
  int lineno; // line number
};

class Attribute {
public:
  string owner;
  string attr;
  string type;
  int lineno;
  Attribute(string t, string o, string a) : owner(o), attr(a), type(t) {}
};

} // namespace VL2XML_PARSER

#endif