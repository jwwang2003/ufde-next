#pragma once
#ifndef _RT_CILLIB_H_
#define _RT_CILLIB_H_

#include "arch/archlib.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range/algorithm.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <rapidxml/rapidxml_utils.hpp>

namespace FDU {
namespace rt_cil_lib {
using namespace ARCH;

// ElementBase.h
class CilBase : boost::noncopyable {
protected:
  std::string _name;

public:
  virtual ~CilBase() = 0;
  CilBase(const std::string &name) : _name(name) {}
  std::string getName() const { return _name; }
};

// Container.h
using std::string;

template <typename T> struct matchName {
  matchName(const string &name) : _toMatch(name) {}
  bool operator()(const T &t) { return _toMatch == t.getName(); }

private:
  string _toMatch;
};

template <typename R, typename T, typename C = boost::ptr_vector<T>>
struct pContainer : public C {
  template <typename IT, typename Rng = boost::iterator_range<IT>>
  struct iter_pair : Rng {
    iter_pair(IT begin, IT end) : Rng(begin, end) {}

    typename std::iterator_traits<IT>::pointer find(const string &name) const {
      IT it = boost::find_if(*this, matchName<R>(name));
      if (it == (*this).end()) {
        //				std::cout << Warning("container: no such
        // member ... " + name) << std::endl;
        return nullptr;
      } else
        return &*it;
      //			ASSERT(it != end(), FDU::Exception("container:
      // no such member ..." + name));
    }
  };

  using typename C::const_iterator;
  using typename C::iterator;

  typedef iter_pair<iterator> range_type;
  typedef iter_pair<const_iterator> const_range_type;

  template <typename PT> T &add(PT t) {
    T &rt = *t;
    this->push_back(t);
    return rt;
  } // PT is T* or std::unique_ptr<T>
    // 	template<typename PT> std::pair<T*, iterator> insert(iterator it, PT t)
    // 	{ T* rt = &*t; it = C::insert(it, t); return std::make_pair(rt, it); }

  range_type range() { return range_type((*this).begin(), (*this).end()); }
  const_range_type range() const {
    return const_range_type((*this).begin(), (*this).end());
  }
};

template <typename T>
struct cilContainer : pContainer<FDU::rt_cil_lib::CilBase, T> {};

// sramInElem.h
class sramElem : public CilBase {
private:
  bool _default;
  int _content;

public:
  sramElem(bool defaultORnot, int content, const std::string &sramName)
      : CilBase(sramName), _default(defaultORnot), _content(content) {}

  bool getdefault() { return _default; }
  void setContent(int content) { _content = content; }
  int getContent() const { return _content; }
};

class contSramsElem {
public:
  typedef cilContainer<sramElem>::range_type sramsElemType;
  typedef cilContainer<sramElem>::const_range_type const_sramsElemType;
  typedef cilContainer<sramElem>::iterator sramElemIter;
  typedef cilContainer<sramElem>::const_iterator const_sramElemIter;

private:
  cilContainer<sramElem> _srams;

public:
  sramsElemType srams() { return _srams.range(); }
  const_sramsElemType srams() const { return _srams.range(); }

  sramElem &addSram(sramElem *sram) { return _srams.add(sram); }
  sramElem &getSram(const std::string &sramName) {
    return *srams().find(sramName);
  }
};

// pathInElem.h
class pathElem : public CilBase {
private:
  std::string _in;
  std::string _out;
  bool _segregated;
  contSramsElem _cfgInfo;

public:
  typedef const std::string cstString;
  pathElem(cstString &pathName, cstString &in, cstString &out, bool seg = false)
      : CilBase(pathName), _in(in), _out(out), _segregated(seg) {}

  void setSegregated(bool Segregated) { _segregated = Segregated; }
  bool isSegregated() const { return _segregated; }
  bool searchMe(const std::string &in, const std::string &out) const;

  std::string getIn() { return _in; }
  std::string getOut() { return _out; }

  sramElem &addCfgSram(sramElem *sram) { return _cfgInfo.addSram(sram); }
  sramElem &getCfgSram(const std::string &sramName) {
    return _cfgInfo.getSram(sramName);
  }

  contSramsElem &getCfgInfo() { return _cfgInfo; }
};

class contPathsElem {
public:
  typedef cilContainer<pathElem>::range_type pathsElemType;
  typedef cilContainer<pathElem>::const_range_type const_pathsElemType;
  typedef cilContainer<pathElem>::iterator pathElemIter;
  typedef cilContainer<pathElem>::const_iterator const_pathElemIter;

private:
  cilContainer<pathElem> _paths;

public:
  pathsElemType paths() { return _paths.range(); }
  const_pathsElemType paths() const { return _paths.range(); }

  pathElem &addPath(pathElem *path) { return _paths.add(path); }
  pathElem &getPath(const std::string &pathName) {
    return *paths().find(pathName);
  }

  bool hasRequiredPath(const std::string &in, const std::string &out);
};

// Element.h
class Element : public CilBase {
public:
  typedef contSramsElem Srams;
  typedef contPathsElem Paths;

private:
  Srams _srams;
  Paths _paths;

public:
  Element(const std::string &elemName) : CilBase(elemName) {}

  Srams &getSrams() { return _srams; }
  Paths &getPaths() { return _paths; }

  bool hasRequiredPath(const std::string &in, const std::string &out);
};

inline bool Element::hasRequiredPath(const std::string &in,
                                     const std::string &out) {
  return _paths.hasRequiredPath(in, out);
}

// elemLib.h
class elemLib {
public:
  typedef cilContainer<Element>::range_type elementsType;
  typedef cilContainer<Element>::const_range_type const_elementsType;
  typedef cilContainer<Element>::iterator elementIter;
  typedef cilContainer<Element>::const_iterator const_elementIter;

private:
  cilContainer<Element> _elements;

public:
  elementsType elements() { return _elements.range(); }
  const_elementsType elements() const { return _elements.range(); }

  Element &addElement(Element *elem) { return _elements.add(elem); }
  Element *getElement(const std::string &elemName) {
    return elements().find(elemName);
  }
};

class cilLibrary {
private:
  elemLib _elemLibrary;

public:
  static cilLibrary *loadCilLib(const std::string &file);

  elemLib *getElemLib() { return &_elemLibrary; }
};

} // namespace rt_cil_lib
} // namespace FDU

#endif