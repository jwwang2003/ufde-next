#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include <iostream>
#include <string>
#include <vector>
// #include <functional>
// #include <boost/foreach.hpp>
// #include <boost/range/algorithm.hpp>
// #include <boost/ptr_container/ptr_vector.hpp>

#include "cil/cilBase.h"
// #include "circuit/cktBase.h"
#include "bitstream/bstrBase.h"
#include "exception/exceptions.h"
#include "ptrcontainer.hpp"

using std::string;
/*
template<typename T>
struct matchName : public std::unary_function<T, bool> {
        matchName(const string& name) : _toMatch(name) {}
        bool operator() (const T& t) { return _toMatch == t.getName(); }
private:
        string _toMatch;
};

template<typename R, typename T, typename C = boost::ptr_vector<T> > struct
pContainer : public C { template <typename IT, typename Rng =
boost::iterator_range<IT> > struct iter_pair : Rng { iter_pair(IT begin, IT end)
: Rng(begin, end) {}

                typename std::iterator_traits<IT>::pointer find(const string&
name) const { IT it = boost::find_if(*this, matchName<R>(name)); if(it ==
end()){
//				std::cout << Warning("container: no such member
... " + name) << std::endl; return nullptr;
                        }
                        else
                                return &*it;
//			ASSERT(it != end(), FDU::Exception("container: no such
member ..." + name));
                }
        };

        using typename C::iterator;
        using typename C::const_iterator;

        typedef iter_pair<iterator>        range_type;
        typedef iter_pair<const_iterator>  const_range_type;

        template<typename PT> T& add(PT t) { T& rt = *t; push_back(t); return
rt; }		// PT is T* or std::auto_ptr<T>
// 	template<typename PT> std::pair<T*, iterator> insert(iterator it, PT t)
// 	{ T* rt = &*t; it = C::insert(it, t); return std::make_pair(rt, it); }

        range_type       range()       { return range_type(begin(), end()); }
        const_range_type range() const { return const_range_type(begin(),
end()); }
};
*/
template <typename T>
struct cilContainer : COS::PtrContainer<T, std::vector<T *>, true> {};
template <typename T>
struct cktContainer : COS::PtrContainer<T, std::vector<T *>, true> {};
template <typename T>
struct bstrContainer : COS::PtrContainer<T, std::vector<T *>, true> {};
#endif