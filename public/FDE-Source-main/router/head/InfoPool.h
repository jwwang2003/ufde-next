#ifndef _INFOPOOL_H
#define _INFOPOOL_H

/*#include "RTNode.h"*/

#include "tnetlist.hpp"
namespace FDU {
namespace RT {

using COS::TDesign;
class RTGraph;

class InfoPool {
public:
  InfoPool() : p_rrg_(nullptr), p_design_(nullptr) {}

  void set_rrg(const RTGraph *p_rrg) { p_rrg_ = p_rrg; }
  void set_design(const TDesign *p_design) { p_design_ = p_design; }

  bool check_netlist() const;
  void echo_netlist() const;
  void echo_arch() const;
  void echo_success_rt(int) const;

private:
  const RTGraph *p_rrg_;
  const TDesign *p_design_;
};
} // namespace RT
} // namespace FDU

#endif