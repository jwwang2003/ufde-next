#include "PKUtils.h"

namespace PACK {

using namespace boost;

StrVec &split_specific(const string &s, StrVec &svec, const string &delim) {
  svec.clear();
  string::size_type p = s.find(delim);
  if (p == string::npos)
    svec.push_back(s);
  else {
    svec.push_back(s.substr(0, p));
    svec.push_back(s.substr(p + delim.length()));
  }
  return svec;
}

StrVec &split_statement(const string &s, StrVec &svec) {
  string temp = trim_copy(s);
  return split(svec, temp, is_any_of(" "), token_compress_on);
}

StrVec &split_params(const string &sparam, StrVec &svec) {
  string temp = trim_copy_if(sparam, is_any_of("()"));
  return split(svec, temp, is_any_of(","));
}

bool match_fake_name(const string &sname, smatch &m) {
  static regex reg("(.*)\\{\\d+\\}(.*)");
  return regex_match(sname, m, reg);
}

bool is_fake_name(const string &sname) {
  static regex reg("(.*)\\{\\d+\\}(.*)");
  return regex_match(sname, reg);
}

string hex2expr(const string &hex, int inum) {
  const int MAXI = 4;
  static const char *const item[MAXI][(1 << MAXI) + 1] = {
      {"~A1*A1", "A1", "~A1"},
      {"(~A1*A1)+(~A2*A2)", "(A2*A1)", "(A2*~A1)", "(~A2*A1)", "(~A2*~A1)"},
      {"(~A1*A1)+(~A2*A2)+(~A3*A3)", "((A3*A2)*A1)", "((A3*A2)*~A1)",
       "((A3*~A2)*A1)", "((A3*~A2)*~A1)", "((~A3*A2)*A1)", "((~A3*A2)*~A1)",
       "((~A3*~A2)*A1)", "((~A3*~A2)*~A1)"},
      {"(~A1*A1)+(~A2*A2)+(~A3*A3)+(~A4*A4)", "(((A4*A3)*A2)*A1)",
       "(((A4*A3)*A2)*~A1)", "(((A4*A3)*~A2)*A1)", "(((A4*A3)*~A2)*~A1)",
       "(((A4*~A3)*A2)*A1)", "(((A4*~A3)*A2)*~A1)", "(((A4*~A3)*~A2)*A1)",
       "(((A4*~A3)*~A2)*~A1)", "(((~A4*A3)*A2)*A1)", "(((~A4*A3)*A2)*~A1)",
       "(((~A4*A3)*~A2)*A1)", "(((~A4*A3)*~A2)*~A1)", "(((~A4*~A3)*A2)*A1)",
       "(((~A4*~A3)*A2)*~A1)", "(((~A4*~A3)*~A2)*A1)",
       "(((~A4*~A3)*~A2)*~A1)"}};
  if (inum == 1)
    inum = 0;
  else { // recalculate inum
    inum = 1;
    for (size_t len = 1; len < hex.length(); len *= 2)
      inum++;
  }
  ASSERT(inum < MAXI, "The number of lut inputs exceeds " << MAXI);

  string expr;
  for (int i = 0, n = 1; i < hex.length(); i++) {
    int hn = hex[i] & 0xf;
    if (hex[i] > '9')
      hn += 9;
    for (int m = inum ? 8 : 2; m; m >>= 1, n++)
      if (hn & m)
        expr.append("+").append(item[inum][n]);
  }
  if (expr.empty())
    return item[inum][0];
  return expr.substr(1);
}

string exchange_expr(string s, const string &a, const string &b) {
  static regex var_ex("A[1234]");
  ASSERTSD(regex_match(a, var_ex) && regex_match(b, var_ex));
  ASSERTSD(s.find(a) != string::npos && s.find(b) != string::npos);
  for (int i = 0; i < s.length(); i++) {
    if (s[i] == a[1])
      s[i] = b[1];
    else if (s[i] == b[1])
      s[i] = a[1];
  }
  return s;
}

string sub_expr(string s, const string &a, const string &b) {
  static regex var_ex("A[1234]");
  ASSERTSD(regex_match(a, var_ex) && regex_match(b, var_ex));
  ASSERTSD(s.find(a) != string::npos && s.find(b) == string::npos);
  for (int i = 0; i < s.length(); i++)
    if (s[i] == a[1])
      s[i] = b[1];
  return s;
}

} // namespace PACK
