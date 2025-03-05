#ifndef _ELEMLIB_H_
#define _ELEMLIB_H_

#include "cil/elementLib/element/Element.h"
#include "container/Container.h"

namespace FDU {
namespace cil_lib {

class elemLib {
public:
  using elementsType = cilContainer<Element>::range_type;
  using const_elementsType = cilContainer<Element>::const_range_type;
  using elementIter = cilContainer<Element>::iterator;
  using const_elementIter = cilContainer<Element>::const_iterator;

private:
  cilContainer<Element> _elements;

public:
  elementsType elements() { return _elements.range(); }
  const_elementsType elements() const { return _elements.range(); }

  Element *addElement(Element *elem) { return _elements.add(elem); }
  Element *getElement(const std::string &elemName) {
    return elements().find(elemName);
  }
};

} // namespace cil_lib
} // namespace FDU

#endif