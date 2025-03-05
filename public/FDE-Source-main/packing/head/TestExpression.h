#ifndef _TEST_EXPRESSION_H
#define _TEST_EXPRESSION_H

#include "PKUtils.h"

namespace PACK {

class TranTestCase;

enum TExprType {
  UKWN_EXPR = -1,
  TTRUE,
  TFANIN_TYPE,
  TSLICE_FULL,
  TPIN_USED,
  TINST_USED,
  TCURRENT_PROCESS,
  NUM_TEST_EXPR
};

std::istream &operator>>(std::istream &s, TExprType &itype);

class TestExprParser {
  typedef void (*PTEHandler)(const string &, TranTestCase *, bool);

public:
  static void parse_test_expr(const string &, TranTestCase *);

public:
  static PTEHandler parsing_array[NUM_TEST_EXPR];
  // static boost::smatch regex_m;
};

class TestExpr {
  friend class TestExprParser;

public:
  // virtual string name()    const = 0;
  virtual bool execute() const = 0;
  virtual ~TestExpr() {}

protected:
  TestExpr(TranTestCase *c, bool is_inverted)
      : is_inverted_(is_inverted), co_case_(c) {}

protected:
  bool is_inverted_;
  TranTestCase *co_case_;
};

class TTrue : public TestExpr {
  friend class TestExprParser;

public:
  TTrue(TranTestCase *c, bool is_inverted) : TestExpr(c, is_inverted) {}

  // string name()    const { return TEXPR_NAME; }
  bool execute() const;

private:
  static void parse(const string &, TranTestCase *, bool);

public:
  static const int NUM_PARAM;
  static const char *const TEXPR_NAME;
};

class TFaninType : public TestExpr {
  friend class TestExprParser;

public:
  TFaninType(TranTestCase *c, bool is_inverted) : TestExpr(c, is_inverted) {}

  // string name()    const { return TEXPR_NAME; }
  bool execute() const;

private:
  static void parse(const string &, TranTestCase *, bool);

private:
  NameInfo iname_i_;
  string pname_;
  std::vector<string> tnames_;

public:
  static const int NUM_PARAM;
  static const char *const TEXPR_NAME;
};

class TSliceFull : public TestExpr {
  friend class TestExprParser;

public:
  TSliceFull(TranTestCase *c, bool is_inverted) : TestExpr(c, is_inverted) {}

  // string name()    const { return TEXPR_NAME; }
  bool execute() const;

private:
  static void parse(const string &, TranTestCase *, bool);

public:
  static const int NUM_PARAM;
  static const char *const TEXPR_NAME;
};

class TPinUsed : public TestExpr {
  friend class TestExprParser;

public:
  TPinUsed(TranTestCase *c, bool is_inverted) : TestExpr(c, is_inverted) {}

  // string name()    const { return TEXPR_NAME; }
  bool execute() const;

private:
  static void parse(const string &, TranTestCase *, bool);

private:
  NameInfo iname_i_;
  string pname_;

public:
  static const int NUM_PARAM;
  static const char *const TEXPR_NAME;
};

///////////sophie////////////////
class TInstUsed : public TestExpr {
  friend class TestExprParser;

public:
  TInstUsed(TranTestCase *c, bool is_inverted) : TestExpr(c, is_inverted) {}

  bool execute() const;

private:
  static void parse(const string &, TranTestCase *, bool);

private:
  NameInfo cname_i_;
  NameInfo iname_i_;

public:
  static const int NUM_PARAM;
  static const char *const TEXPR_NAME;
};
///////////sophie////////////////
class TCurrentProcess : public TestExpr {
  friend class TestExprParser;

public:
  TCurrentProcess(TranTestCase *c, bool is_inverted)
      : TestExpr(c, is_inverted) {}

  bool execute() const;

private:
  static void parse(const string &, TranTestCase *, bool);

private:
  NameInfo iname_i_;
  string prop_name_;
  string prop_value_;

public:
  static const int NUM_PARAM;
  static const char *const TEXPR_NAME;
};
} // namespace PACK

#endif