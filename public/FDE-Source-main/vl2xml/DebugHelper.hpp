#ifndef _DEBUGHELPER_HPP_
#define _DEBUGHELPER_HPP_
#include <iostream>
#include <map>
#include <string>
#include <vector>

extern std::vector<std::string> stepStack;

#define PUSH_STEP stepStack.push_back(__func__)
#define PUSH_STEP_NAMED(x) stepStack.push_back(x)
#define POP_STEP stepStack.pop_back()

std::string NotFindExp(std::string type, std::string name);
void ReportError(std::string log, int lv);

template <typename T>
void CheckPointer(T pointer, std::string type, std::string name) {
  if (pointer == nullptr) {
    ReportError(NotFindExp(type, name), 0);
  }
}

void CheckMapItem(std::map<std::string, int> const &map, std::string type,
                  std::string name);
#endif
