#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <boost/lexical_cast.hpp>
#include <map>
#include <stdint.h>
#include <string>

#include "log.h"
#include "ptrcontainer.hpp"
#include "utils.h"

namespace COS {
using std::string;
using namespace FDU;

enum ObjectClass {
  OBJECT,
  DESIGN,
  LIBRARY,
  MODULE,
  INSTANCE,
  PORT,
  NET,
  PIN,
  BUS
};
enum StorageType { TEMPORARY = 0, COPY = 1, SAVE = 2, COPY_SAVE = 3 };

class Object;
template <typename T> class Property;
class Config;

class PropertyBase : boost::noncopyable {
public:
  PropertyBase(const string &name, StorageType st) : _name(name), _store(st) {}
  virtual ~PropertyBase() {}

  const string &name() const { return _name; }
  bool store_save() const { return _store & SAVE; }
  bool store_copy() const { return _store & COPY; }

  bool operator==(const PropertyBase &rhs) const { return this == &rhs; }
  bool operator!=(const PropertyBase &rhs) const { return this != &rhs; }

  virtual void copy(const Object *to, const Object *from) = 0;
  virtual bool exist(const Object *obj) const = 0;
  virtual void set_string_value(const Object *obj, const string &value) {}
  virtual string string_value(const Object *obj) const {
    ASSERT(0, "this function shound not be called.");
    return "";
  }
  virtual string value_type() const { return ""; }

private:
  string _name;
  StorageType _store;
};

class PropRepository {
public:
  using property_container = PtrVector<PropertyBase>;
  using properties_type = property_container::range_type;
  using property_iter = property_container::iterator;

  using config_container = PtrVector<Config>;
  using configs_type = config_container::range_type;
  using config_iter = config_container::iterator;

  properties_type get_properties(ObjectClass obj_class) {
    return _propRepository[obj_class].range();
  }

  PropertyBase *find_property(ObjectClass obj_class, const string &name) {
    return get_properties(obj_class).find(name);
  }

  template <typename T>
  Property<T> &create_property(ObjectClass obj_class, const string &name,
                               const T &def_val, StorageType st);
  template <typename T>
  Property<T> &create_temp_property(ObjectClass obj_class, const string &name,
                                    const T &def_val, StorageType st);

  configs_type get_configs(const string &inst_type) {
    return _configRepository[inst_type].range();
  }

  Config *find_config(const string &inst_type, const string &name) {
    return get_configs(inst_type).find(name);
  }

  Config &create_config(const string &inst_type, const string &name,
                        const string &def_val);

  static PropRepository &instance() { // singleton instance
    static PropRepository _instance;
    return _instance;
  }

private:
  std::map<ObjectClass, property_container> _propRepository;
  std::map<string, config_container> _configRepository;
};

template <typename T> struct prop_type_traits {
  static const char *type_string() { return ""; }
};
template <> struct prop_type_traits<int> {
  static const char *type_string() { return "int"; }
};
template <> struct prop_type_traits<int64_t> { // for Yosys edif
  static const char *type_string() { return "int"; }
};
template <> struct prop_type_traits<double> {
  static const char *type_string() { return "double"; }
};
template <> struct prop_type_traits<string> {
  static const char *type_string() { return "string"; }
};
template <> struct prop_type_traits<Point> {
  static const char *type_string() { return "point"; }
};

template <typename T> class Property : public PropertyBase {
public:
  explicit Property(const T &def_val = T())
      : PropertyBase("", TEMPORARY), _def_val(def_val) {} // temporary property
  using map_iter = typename std::map<const Object *, T>::iterator;
  using const_map_iter = typename std::map<const Object *, T>::const_iterator;

  T get_value(const Object *obj) const {
    const_map_iter p = _prop_map.find(obj);
    return p != _prop_map.end() ? p->second : _def_val;
  }
  T *get_ptr(const Object *obj) {
    map_iter p = _prop_map.find(obj);
    return p != _prop_map.end() ? &p->second : 0;
  }
  const T *get_ptr(const Object *obj) const {
    const_map_iter p = _prop_map.find(obj);
    return p != _prop_map.end() ? &p->second : 0;
  }
  T &get_ref(const Object *obj) {
    map_iter p = _prop_map.find(obj);
    if (p == _prop_map.end())
      set_value(obj, _def_val);
    return _prop_map[obj];
  }
  const T &get_cref(const Object *obj) const {
    const_map_iter p = _prop_map.find(obj);
    return p != _prop_map.end() ? p->second : _def_val;
  }
  void set_value(const Object *obj, const T &value) { _prop_map[obj] = value; }
  //		template<typename... A>
  //		void emplace_value(const Object* obj, A&&... vals)
  //		{ _prop_map.try_emplace(obj, std::foward<A>(vals)...); }

  void copy(const Object *to, const Object *from) {
    map_iter p = _prop_map.find(from);
    if (p != _prop_map.end())
      set_value(to, p->second);
  }
  bool remove(const Object *obj) { return _prop_map.erase(obj); }
  bool exist(const Object *obj) const { return get_ptr(obj) != 0; }
  void clear() { return _prop_map.clear(); }

protected:
  Property(const string &name, const T &def_val, StorageType st)
      : PropertyBase(name, st), _def_val(def_val) {}
  //		friend Property<T>&
  // PropRepository::create_temp_property(ObjectClass obj_class, const string&
  // name, const T& def_val, StorageType st);
  friend class PropRepository;

private:
  T _def_val;
  std::map<const Object *, T> _prop_map;
};

template <typename T>
class RProperty : public Property<T> { // Repository Property, T must support
                                       // stream input/output
public:
  void set_string_value(const Object *obj, const string &value) {
    Property<T>::set_value(obj, boost::lexical_cast<T>(value));
  }
  string string_value(const Object *obj) const {
    return boost::lexical_cast<string>(Property<T>::get_value(obj));
  }
  string value_type() const { return prop_type_traits<T>::type_string(); }

protected:
  RProperty(const string &name, const T &def_val, StorageType st)
      : Property<T>(name, def_val, st) {}
  //		friend Property<T>& PropRepository::create_property(ObjectClass
  // obj_class, const string& name, const T& def_val, StorageType st);
  friend class PropRepository;
};

// specialization for Property<string>
template <>
inline void RProperty<string>::set_string_value(const Object *obj,
                                                const string &value) {
  set_value(obj, value);
}
template <>
inline string RProperty<string>::string_value(const Object *obj) const {
  return get_value(obj);
}

class Config : public RProperty<string> {
private: // cannot create temporary config
  Config(const string &name, const string &def_val)
      : RProperty<string>(name, def_val, COPY_SAVE) {}
  friend Config &PropRepository::create_config(const string &inst_type,
                                               const string &name,
                                               const string &def_val);
};

// repository operations
inline PropRepository::properties_type get_properties(ObjectClass obj_class) {
  return PropRepository::instance().get_properties(obj_class);
}

inline PropertyBase *find_property(ObjectClass obj_class, const string &name) {
  return PropRepository::instance().find_property(obj_class, name);
}

template <typename T>
Property<T> &PropRepository::create_property(ObjectClass obj_class,
                                             const string &name,
                                             const T &def_val, StorageType st) {
  auto pb = find_property(obj_class, name);
  if (auto p = dynamic_cast<RProperty<T> *>(pb))
    return *p;
  ASSERT(!pb, name << " has been created as temporary property");
  return static_cast<Property<T> &>(
      *_propRepository[obj_class].add(new RProperty<T>(name, def_val, st)));
}

template <typename T>
Property<T> &
PropRepository::create_temp_property(ObjectClass obj_class, const string &name,
                                     const T &def_val, StorageType st) {
  if (auto p = dynamic_cast<Property<T> *>(find_property(obj_class, name)))
    return *p;
  return static_cast<Property<T> &>(
      *_propRepository[obj_class].add(new Property<T>(name, def_val, st)));
}

template <typename T>
inline Property<T> &create_property(ObjectClass obj_class, const string &name,
                                    const T &def_val = T(),
                                    StorageType st = COPY_SAVE) {
  return PropRepository::instance().create_property(obj_class, name, def_val,
                                                    st);
}

template <typename T>
inline Property<T> &
create_temp_property(ObjectClass obj_class, const string &name,
                     const T &def_val = T(), StorageType st = TEMPORARY) {
  ASSERT(!(st & SAVE), "temporary property cannot save to file");
  return PropRepository::instance().create_temp_property(obj_class, name,
                                                         def_val, st);
}

inline PropRepository::configs_type get_configs(const string &inst_type) {
  return PropRepository::instance().get_configs(inst_type);
}

inline Config *find_config(const string &inst_type, const string &name) {
  return PropRepository::instance().find_config(inst_type, name);
}

inline Config &PropRepository::create_config(const string &inst_type,
                                             const string &name,
                                             const string &def_val) {
  if (Config *c = find_config(inst_type, name))
    return *c;
  return *_configRepository[inst_type].add(new Config(name, def_val));
}

inline Config &create_config(const string &inst_type, const string &name,
                             const string &def_val = "#OFF") {
  return PropRepository::instance().create_config(inst_type, name, def_val);
}

} // namespace COS

#endif
