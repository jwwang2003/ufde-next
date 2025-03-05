#ifndef LOADARCHLIB_HPP
#define LOADARCHLIB_HPP

#include "arch/archlib.hpp"
#include "io/xml/loadxml.hpp"

namespace COS {
namespace XML {

using namespace ARCH;

class ArchLibHandler : public NetlistHandler {
public:
  Design *load_design(xml_node *node, Design *design);

protected:
  Module *load_module(xml_node *node, Library *lib);
  Port *load_port(xml_node *node, Module *mod);
  Instance *load_instance(xml_node *node, Module *mod);

private:
  void load_info(xml_node *node, FPGADesign *arch);
  void load_package(xml_node *node, FPGADesign *arch);
  void load_path(xml_node *node, ArchCell *cell);
};

} // namespace XML
} // namespace COS

#endif
