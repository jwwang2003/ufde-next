#ifndef PTRCONTAINER_HPP
#define PTRCONTAINER_HPP

#include <boost/range.hpp>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace COS {
using std::string;

///////////////////////////////////////////////////////////////////////////////////////
//  pointer container and range type

template <typename T, typename C /* = vector<T*>*/, bool OWNER>
struct PtrContainer : public C {
  template <typename IT> struct iter_t : IT {
    using pointer = T *;
    using const_pointer = T *const;
    using IT::operator*;

    iter_t() : IT() {}
    iter_t(IT it) : IT(it) {}
    pointer operator->() const { return operator*(); } // auto dereference
  };

  using iterator = iter_t<typename C::iterator>;
  using const_iterator = iter_t<typename C::const_iterator>;
  using C::begin;
  using C::end;

  template <typename IT, typename Rng = boost::iterator_range<IT>>
  struct iter_pair : Rng {
    using Rng::begin;
    using Rng::end;
    iter_pair(IT begin, IT end) : Rng(begin, end) {}
    template <typename R>
    iter_pair(const R &rng) : Rng(rng.begin(), rng.end()) {}

    typename std::iterator_traits<IT>::pointer find(const string &name) const {
      IT it = find_if(begin(), end(),
                      [&name](const T *t) { return name == t->name(); });
      return it == end() ? nullptr : *it;
    }
  };

  using range_type = iter_pair<iterator>;
  using const_range_type = iter_pair<const_iterator>;

  range_type range() { return range_type(begin(), end()); }
  const_range_type range() const { return const_range_type(begin(), end()); }

  template <typename IT> std::pair<T *, IT> insert(IT it, T *t) {
    it = C::insert(it, t);
    return std::make_pair(t, it);
  }

  T *add(T *t) {
    C::insert(end(), t);
    return t;
  }

  template <typename IT> IT erase(IT it) {
    if (OWNER)
      delete *it;
    return C::erase(it);
  }
  template <typename IT> IT erase(IT begin, IT end) {
    if (OWNER) {
      for (IT it = begin; it != end; ++it)
        delete *it;
    }
    return C::erase(begin, end);
  }

  bool erase(T *t) {
    iterator it = find(begin(), end(), t);
    if (it == end())
      return false;
    erase(it);
    return true;
  }

  void clear() {
    delete_all();
    C::clear();
  }

  ~PtrContainer() { delete_all(); }

  // template iterator & range
  template <typename DT> struct typed {
    template <typename IT> struct iter_t : IT {
      using value_type = DT *;
      using pointer = DT *;
      using const_pointer = DT *const;
      using reference = DT *;
      using const_reference = DT *const;

      iter_t() : IT() {}
      iter_t(IT it) : IT(it) {}
      pointer operator*() const {
        return static_cast<pointer>(IT::operator*());
      }
      pointer operator->() const { return operator*(); }
      iter_t operator++() { return static_cast<iter_t>(IT::operator++()); }
      iter_t operator++(int) { return static_cast<iter_t>(IT::operator++(0)); }
    };

    using iterator = iter_t<typename C::iterator>;
    using const_iterator = iter_t<typename C::const_iterator>;
    using range = iter_pair<iterator>;
    using const_range = iter_pair<const_iterator>;
  };

  template <typename DT> typename typed<DT>::range range() {
    return typed<DT>::range(begin(), end());
  }
  template <typename DT> typename typed<DT>::const_range range() const {
    return typed<DT>::const_range(begin(), end());
  }

private:
  void delete_all() {
    if (OWNER)
      for (iterator it = begin(); it != end(); ++it)
        delete *it;
  }
};

template <typename T, bool OWNER = true>
using PtrVector = PtrContainer<T, std::vector<T *>, OWNER>;
template <typename T, bool OWNER = true>
using PtrList = PtrContainer<T, std::list<T *>, OWNER>;

template <typename T, typename C = std::less<T *>, bool OWNER = true>
struct PtrSet : PtrContainer<T, std::set<T *, C>, OWNER> {
  using SET = std::set<T *, C>;
  //		typedef  std::set<T*, C, std::allocator<void*> >	SET;

  using iterator = typename PtrContainer<T, std::set<T *, C>, OWNER>::iterator;
  using const_iterator =
      typename PtrContainer<T, std::set<T *, C>, OWNER>::const_iterator;
  using range_type =
      typename PtrContainer<T, std::set<T *, C>, OWNER>::range_type;
  using const_range_type =
      typename PtrContainer<T, std::set<T *, C>, OWNER>::const_range_type;

  std::pair<iterator, bool> insert(T *t) { return SET::insert(t); }

  T *add(T *t) { return insert(t).second ? t : 0; }

  bool erase(T *t) {
    if (SET::erase(t)) {
      if (OWNER)
        delete t;
      return true;
    }
    return false;
  }
};

} // namespace COS

#endif
