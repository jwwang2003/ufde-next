#include "DebugHelper.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::string;

string NotFindExp(string type, string name) {
  return "There is no [" + type + "] named \"" + name + "\"";
}

void ReportError(string log, int lv) {
  cout << "An error detected in:" << endl;
  for (int i = 0; i < stepStack.size(); i++) {
    for (int j = 0; j < i + 1; j++)
      cout << "    ";
    cout << stepStack[i] << endl;
  }
  cout << "For reason: " << endl;
  cout << "    " << log << endl;
  cout << "Action:" << endl;
  if (lv == 0) {
    cout << "    exit for safety" << endl;
    exit(1);
  } else if (lv == 1) {
    cout << "    continue(Y/N)?";
    string str = "";
    cin >> str;
    if (str != "Y")
      exit(1);
  } else {
    cout << "    continue" << endl;
  }
}

void CheckMapItem(std::map<std::string, int> const &map, std::string type,
                  std::string name) {
  if (map.find(name) == map.end()) {
    ReportError(NotFindExp(type, name), 1);
  }
}
