#ifndef MAPPING_HPP
#define MAPPING_HPP

#include "args.hpp"
#include "netlist.hpp"

class MappingManager {
  Args &_args;
  COS::Design *_pDesign;

public:
  explicit MappingManager(Args &args)
      : _args(args), _pDesign(new COS::Design("map design")) {}

  void doReadDesign();
  void doAigTransform();
  void doMapCut();
  void doPtnMatch();
  void doWriteDesign();
  void doReport();
};

#endif