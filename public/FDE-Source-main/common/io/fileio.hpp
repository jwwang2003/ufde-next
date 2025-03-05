#ifndef FILEIO_HPP
#define FILEIO_HPP

#include <iostream>
#include <string>
#include <vector>

namespace COS {

class Design;
class Module;
class Library;

namespace IO {

///////////////////////////////////////////////////////////////////////////////////////
//  Base classes

class Loader {
public:
  Loader(const std::string &type);
  virtual ~Loader();
  virtual void load(std::istream &istrm) = 0;

  void set_design(Design *design) { _design = design; }
  void load(const std::string &file);

protected:
  Design *design() const { return _design; }

private:
  std::string _type;
  Design *_design;
};

class Writer {
public:
  Writer(const std::string &type);
  virtual ~Writer();
  virtual void write(std::ostream &ostrm) const = 0;

  void set_design(const Design *design) { _design = design; }
  void write(const std::string &file, bool encrypt) const;

  static const std::vector<Library *> &used_lib(const Design *design);
  static const std::vector<Module *> &used_module(const Library *lib);

protected:
  const Design *design() const { return _design; }
  void mark_used() const;
  static void mark_lib(const Library *lib);

private:
  std::string _type;
  const Design *_design;
};

///////////////////////////////////////////////////////////////////////////////////////

Loader *get_loader(std::string type, Design *design);
Writer *get_writer(std::string type, Design *design);

} // namespace IO
} // namespace COS

#endif
