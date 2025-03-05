#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <iostream>

#include "property.hpp"

namespace COS {
using std::string;
using namespace FDU;

///////////////////////////////////////////////////////////////////////////////////////
//  base class for all COS object

class Object : boost::noncopyable {
  string _name;
  ObjectClass _class;
  Object *_owner;

protected:
  Object(const string &name, ObjectClass class_id, Object *owner)
      : _name(name), _class(class_id), _owner(owner) {}
  virtual ~Object() {}
  Object *clone(Object *new_owmer) const {
    Object *new_obj = do_clone(new_owmer);
    new_obj->copy_property(this);
    return new_obj;
  }
  virtual Object *do_clone(Object *new_owmer) const {
    ASSERTD(0, "this function shound not be called.");
    return 0;
  }

public:
  const string &name() const { return _name; }
  virtual void rename(const string &name) { _name = name; }
  virtual string path_name() const { return _name; }
  Object *owner() const { return _owner; }
  ObjectClass class_id() const { return _class; }

  PropRepository::properties_type properties() const {
    return get_properties(class_id());
  }
  virtual PropertyBase *find_property(const string &name) const {
    return COS::find_property(class_id(), name);
  }

  void set_property(Property<string> &p, const char *value) const {
    p.set_value(this, value);
  }
  template <typename T>
  void set_property(Property<T> &p, const T &value) const {
    p.set_value(this, value);
  }
  //		template<typename T, typename... A>
  //		void emplace_property(Property<T>& p, A&& ... vals) const {
  // p.emplace_value(this, std::foward<A>(vals)...); }
  template <typename T> T property_value(const Property<T> &p) const {
    return p.get_value(this);
  }
  template <typename T> T *property_ptr(Property<T> &p) const {
    return p.get_ptr(this);
  }
  template <typename T> const T &property_cref(const Property<T> &p) const {
    return p.get_cref(this);
  }
  template <typename T> T &property_ref(Property<T> &p) const {
    return p.get_ref(this);
  }
  bool property_exist(PropertyBase &p) const { return p.exist(this); }

  void copy_property(const Object *from) const;

  bool operator==(const Object &rhs) const { return this == &rhs; }
  bool operator!=(const Object &rhs) const { return this != &rhs; }
};

inline std::ostream &operator<<(std::ostream &s, const Object &obj) {
  return s << obj.path_name();
}

} // namespace COS

#endif
