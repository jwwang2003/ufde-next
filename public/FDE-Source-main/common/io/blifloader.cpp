#include <boost/algorithm/string.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <list>

#include "blifapp.h"

#include "netlist.hpp"

extern ParseFor parse_for;

namespace {
using namespace std;
using namespace boost;
using namespace boost::lambda;
using namespace FudanFPGA::netlist;

class blif_writer_impl {
  const Design &_design;
  ostream &_os;
  void write_instance(const Instance &inst);

public:
  blif_writer_impl(const Design &design, ostream &os)
      : _design(design), _os(os) {}
  void write_blif();
};

void blif_writer_impl::write_blif() {
  Cell &top_cell = _design.top_cell();
  _os << ".model " << top_cell.name() << endl;

  vector<string> outports;
  string clkport;
  // cell ports
  _os << ".inputs ";

  foreach (const Port &port, top_cell.ports()) {
    //	if (port.direction()==INPUT && port.cell_pin().is_clk())
    if (port.direction() == INPUT && port.cell_pin().net()->type() == CLOCK)
      clkport = port.name();
    if (port.direction() == INPUT)
      _os << port.name() << ' ';
    else if (port.direction() == OUTPUT)
      outports.push_back(port.name());
  }
  _os << endl;
  _os << ".outputs ";
  for (int i = 0; i < outports.size(); ++i)
    _os << outports[i] << ' ';
  _os << endl;

  if (clkport != "")
    _os << ".clock " << clkport << ' ' << endl;

  // cell instances
  for_each(top_cell.instances(),
           bind(&blif_writer_impl::write_instance, this, _1));
  _os << ".end" << endl;
}

void blif_writer_impl::write_instance(const Instance &inst) {
  const string &cell_type = inst.cell_type();
  if (cell_type == "LUT") {
    vector<string> outnets;
    _os << ".names ";
    foreach (const Pin &pin, inst.pins())
      if (pin.direction() == INPUT)
        _os << pin.net()->name() << ' ';
      else if (pin.direction() == OUTPUT)
        outnets.push_back(pin.net()->name());

    for (int i = 0; i < outnets.size(); ++i)
      _os << outnets[i] << ' ';
    _os << endl;
    _os << inst.property_value<string>("init_line", "");
    _os << endl;
  } else if (cell_type == "REG") {
    string clk_net, innet, outnet;
    _os << ".latch ";
    foreach (const Pin &pin, inst.pins())
      if (pin.type() == CLOCK)
        clk_net = pin.net()->name();
      else if (pin.direction() == INPUT)
        innet = pin.net()->name();
      else if (pin.direction() == OUTPUT)
        outnet = pin.net()->name();

    _os << innet << ' ' << outnet << ' '
        << inst.property_value<string>("type", "") << ' ' << clk_net << ' ';
    _os << inst.property_value("init_state", 0) << endl;
  }
}

class blif_loader_impl {
  static const int max_lut = 6;

  Design &_design;
  istream &_is;

  int _inst_count;
  int _net_count;

  vector<string> tokens;
  bool _put_back;

  Cell *LUT[max_lut + 1], *ZERO, *ONE, *IPAD, *OPAD, *BUF, *INV, *FF;
  Cell *_top_cell;
  Net *_vcc, *_gnd;

  void prepare_lib_cells();
  void get_tokens();
  bool get_truth_tokens();
  void put_back_tokens() { _put_back = true; }
  Net &get_net(const string &name);
  Net &get_vcc_net();
  Net &get_gnd_net();
  string calc_init(ParseFor parse_for, size_t lut_size);
  string inst_name(const string &prefix) {
    return prefix + '_' + lexical_cast<string>(_inst_count++);
  }
  Instance &create_instance(Cell *instof) {
    string prefix = instof->name();
    to_lower(prefix);
    return _top_cell->create_instance(inst_name(prefix), *instof);
  }

  void create_model(const string &lib);
  void create_io(PortDir dir, PortType ptype = NORMAL);
  void create_logic();
  void create_latch();
  Cell *create_LUT(int size);

public:
  blif_loader_impl(Design &design, istream &is);
  void load_blif(const string &lib);
};

blif_loader_impl::blif_loader_impl(Design &design, istream &is)
    : _design(design), _is(is), _inst_count(0), _net_count(0), _put_back(false),
      _vcc(0), _gnd(0) {
  prepare_lib_cells();
}

void blif_loader_impl::prepare_lib_cells() {
  Library *cell_lib = _design.libs().find("cell_lib");
  Assert(cell_lib, "blifloader: cell_lib not found");

  string cell_name = "X_LUTn";
  for (int n = 2; n <= max_lut; n++) {
    cell_name[5] = '0' + n;
    LUT[n] = cell_lib->cells().find(cell_name);
    Assert(LUT[n], "blifloader: cell not found: " + cell_name);
  }

  ZERO = cell_lib->cells().find("X_ZERO");
  Assert(ZERO, netlist_error("blifloader: cell not found: X_ZERO"));
  ONE = cell_lib->cells().find("X_ONE");
  Assert(ONE, netlist_error("blifloader: cell not found: X_ONE"));
  /*IPAD = cell_lib->cells().find("X_IPAD");
  Assert(IPAD, netlist_error("blifloader: cell not found: X_IPAD"));
  OPAD = cell_lib->cells().find("X_OPAD");
  Assert(OPAD, netlist_error("blifloader: cell not found: X_OPAD"));*/
  BUF = cell_lib->cells().find("X_BUF");
  Assert(BUF, netlist_error("blifloader: cell not found: X_BUF"));
  INV = cell_lib->cells().find("X_INV");
  Assert(INV, netlist_error("blifloader: cell not found: X_INV"));
  FF = cell_lib->cells().find("X_FF");
  Assert(FF, netlist_error("blifloader: cell not found: X_FF"));
}

void blif_loader_impl::load_blif(const string &lib) {
  while (_is) {
    get_tokens();
    if (tokens.empty())
      continue;
    string keyword = tokens[0];

    if (keyword == ".model")
      create_model(lib);
    else if (keyword == ".inputs")
      create_io(INPUT);
    else if (keyword == ".outputs")
      create_io(OUTPUT);
    else if (keyword == ".clock")
      ;
    else if (keyword == ".names")
      create_logic();
    else if (keyword == ".latch")
      create_latch();
    else if (keyword == ".end")
      break;
    else
      throw netlist_error("blifloader: unknown token: " + keyword);
  }
}

void blif_loader_impl::get_tokens() {
  if (_put_back) {
    _put_back = false;
    return;
  }

  string whole_line;
  while (_is) {
    string line;
    getline(_is, line);
    size_t pos = line.find('#');
    if (pos != string::npos)
      line.erase(pos); // trim comment
    trim(line);
    if (line.empty())
      continue;

    whole_line += line;
    if (ends_with(whole_line, "\\"))
      whole_line.erase(whole_line.size() - 1); // remove '\'
    else {
      trim(whole_line);
      if (!whole_line.empty())
        break;
    }
  }
  tokens.clear();
  split(tokens, whole_line, is_space(), token_compress_on);
}

bool blif_loader_impl::get_truth_tokens() {
  get_tokens();
  Assert(!tokens.empty(), netlist_error("blifloader: truthtable error"));
  char c = tokens[0][0];
  if (c == '0' || c == '1' || c == '-')
    return true;
  put_back_tokens();
  return false;
}

void blif_loader_impl::create_model(const string &lib) {
  Assert(tokens.size() == 2, netlist_error("blifloader: .model error"));
  Library &work_lib = _design.create_library(lib);
  _top_cell = &work_lib.create_cell(tokens[1]);
  _design.set_top_cell(*_top_cell);
}

void blif_loader_impl::create_io(PortDir dir, PortType ptype) {
  for (int i = 1; i < tokens.size(); i++) {
    Port &port = _top_cell->create_port(tokens[i], dir, ptype);
    Net &net = _top_cell->create_net(tokens[i], ptype);
    // Instance& pad = create_instance(dir == INPUT ? IPAD : OPAD);
    port.cell_pin().hookup(net);
    // pad.pins().begin()->hookup(net);
  }
}

void blif_loader_impl::create_latch() {
  static const char *attrlist[] = {"fe", "re", "ah", "al"};
  static const char *rst_pins[] = {"RST", "SET", "", ""};

  Assert(tokens.size() == 6, netlist_error("blifloader: unknown latch format"));

  bool rst_n = false;
  Net *rst = _top_cell->nets().find("rst_n");
  if (rst)
    rst_n = true;
  else
    rst = _top_cell->nets().find("rst");
  if (rst)
    rst->set_type(RESET);

  Net &n_in = get_net(tokens[1]);
  Net &n_out = get_net(tokens[2]);
  Net &n_clk = get_net(tokens[4]);
  n_clk.set_type(CLOCK);

  string &type = tokens[3];
  int init_state = lexical_cast<int>(tokens[5]);

  Instance &instance = create_instance(FF);

  instance.set_property("clock_type", type, 1);
  instance.set_property("init_state", lexical_cast<string>(init_state), 1);

  foreach (Pin &pin, instance.pins()) {
    switch (pin.type()) {
    case NORMAL:
      if (pin.is_sink())
        pin.hookup(n_in);
      else
        pin.hookup(n_out);
      break;
    case CLOCK:
      if (n_clk.name() == "NIL")
        ;
      else
        pin.hookup(n_clk);
      break;
    case ENABLE:
      pin.hookup(get_vcc_net());
      break;
    case RESET:
      if (rst && pin.name() == rst_pins[init_state]) {
        if (rst_n) {
          Instance &inv = create_instance(INV);
          Net &nrst = _top_cell->create_net(inv.name());
          inv.pins().find("I")->hookup(*rst);
          inv.pins().find("O")->hookup(nrst);
          pin.hookup(nrst);
        } else
          pin.hookup(*rst);
      } else
        pin.hookup(get_gnd_net());
      break;
    }
  }
}

void blif_loader_impl::create_logic() {
  int size = tokens.size() - 2;

  // now create lut dynamically when lut size is more than max_lut(basically 6).
  // Assert(size >= 0 && size <= max_lut, netlist_error("blifloader: logic size
  // error\n"));

  vector<string> net_names = tokens;

  // consider that .name gnd. there is no following line
  // Assert(get_truth_tokens(), netlist_error("blifloader: truthtable error
  // 1\n"));
  get_truth_tokens();

  if (size == 0) {
    Assert(tokens.size() == 1,
           netlist_error("blifloader: truthtable error 2\n"));
    switch (tokens[0][0]) {
    case '0': {
      Net *gnd1 = _top_cell->nets().find(net_names[1]);
      Net *gnd2 = &get_gnd_net();
      if (!gnd1)
        gnd2->rename(net_names[1]);
      else if (gnd1 != gnd2)
        gnd1->merge(*gnd2);
      return;
    }
    case '1': {
      Net *vcc1 = _top_cell->nets().find(net_names[1]);
      Net *vcc2 = &get_vcc_net();
      if (!vcc1)
        vcc2->rename(net_names[1]);
      else if (vcc1 != vcc2)
        vcc1->merge(*vcc2);
      return;
    }
    default: {
      if (net_names[1] == "gnd") {
        Net *gnd1 = _top_cell->nets().find(net_names[1]);
        Net *gnd2 = &get_gnd_net();
        if (!gnd1)
          gnd2->rename(net_names[1]);
        else if (gnd1 != gnd2)
          gnd1->merge(*gnd2);
        return;
      } else if (net_names[1] == "vcc") {
        Net *vcc1 = _top_cell->nets().find(net_names[1]);
        Net *vcc2 = &get_vcc_net();
        if (!vcc1)
          vcc2->rename(net_names[1]);
        else if (vcc1 != vcc2)
          vcc1->merge(*vcc2);
        return;
      } else {
        throw netlist_error("blifloader: truthtable error");
      }
    }
    }
  }

  Cell *cell;
  vector<Net *> nets;
  for (int i = 1; i < size + 2; i++)
    nets.push_back(&get_net(net_names[i]));

  if (size == 1) {
    Assert(tokens.size() == 2 || tokens[1] == "1",
           netlist_error("blifloader: truthtable error 3"));
    switch (tokens[0][0]) {
    case '0':
      cell = INV;
      break;
    case '1':
      cell = BUF;
      break;
    default:
      throw netlist_error("blifloader: truthtable error 4");
    }
  } else if (size > max_lut) {
    std::cout << "Warning : LUT size is bigger than 6.\n";
    cell = create_LUT(size);
  } else
    cell = LUT[size];

  Instance &instance = create_instance(cell);
  int i = 0;
  foreach (Pin &pin, instance.pins()) {
    if (pin.is_source())
      pin.hookup(*nets[size]);
    else
      pin.hookup(*nets[i++]);
  }

  if (parse_for == ParseFor::MAPPER) {
    instance.set_property("truthtable", calc_init(ParseFor::MAPPER, size), 1);
  } else if (parse_for == ParseFor::PACKER) {
    if (size > max_lut)
      instance.set_property("output_cover", calc_init(ParseFor::PACKER, size),
                            1);
    else
      instance.set_property("truthtable", calc_init(ParseFor::PACKER, size), 1);
  }
}

Cell *blif_loader_impl::create_LUT(int size) {
  Library *cell_lib = _design.libs().find("cell_lib");
  Cell *cell;
  string lut_name = "X_LUT" + lexical_cast<string>(size);
  if (cell_lib->cells().find(lut_name)) {
    cell = cell_lib->cells().find(lut_name);
  } else {
    cell_lib->create_cell(lut_name, "LUT");
    cell = cell_lib->cells().find(lut_name);

    cell->create_port("O", PortDir::OUTPUT);
    int i = 0;
    while (i < size) {
      string port_name = "ADR" + lexical_cast<string>(i);
      cell->create_port(port_name, PortDir::INPUT);
      i++;
    }
  }
  return cell;
}

Net &blif_loader_impl::get_net(const string &name) {
  if (Net *net = _top_cell->nets().find(name))
    return *net;
  return _top_cell->create_net(name);
}

Net &blif_loader_impl::get_vcc_net() {
  if (!_vcc) {
    Instance &inst = create_instance(ONE);
    _vcc = &_top_cell->create_net(inst.name());
    inst.pins().begin()->hookup(*_vcc);
  }
  return *_vcc;
}

Net &blif_loader_impl::get_gnd_net() {
  if (!_gnd) {
    Instance &inst = create_instance(ZERO);
    _gnd = &_top_cell->create_net(inst.name());
    inst.pins().begin()->hookup(*_gnd);
  }
  return *_gnd;
}

string blif_loader_impl::calc_init(ParseFor parse_for, size_t lut_size) {
  list<string> covers;
  if (parse_for == ParseFor::MAPPER) {
    string output_cover = tokens[0];
    while (get_truth_tokens()) {
      Assert(tokens.size() == 2 || tokens[1] == "1",
             netlist_error("blifloader: truthtable error 5"));
      output_cover += "|" + tokens[0];
    }
    return output_cover;
  } else if (parse_for == ParseFor::PACKER) {
    if (lut_size > max_lut) {
      Assert(tokens.size() == 2 || tokens[1] == "1",
             netlist_error("blifloader: truthtable error 5"));
      string output_cover = tokens[0];
      while (get_truth_tokens()) {
        Assert(tokens.size() == 2 || tokens[1] == "1",
               netlist_error("blifloader: truthtable error 5"));
        output_cover += "|" + tokens[0];
      }
      return output_cover;
    }
    string init(1 << lut_size, '0');
    do {
      Assert(tokens.size() == 2 || tokens[1] == "1",
             netlist_error("blifloader: truthtable error 5"));
      covers.push_back(tokens[0]);
    } while (get_truth_tokens());

    list<string>::iterator it = covers.begin();
    while (it != covers.end()) {
      string s = *it;
      size_t p = s.find('-');
      if (p != string::npos) {
        s[p] = '0';
        covers.push_back(s);
        s[p] = '1';
        covers.push_back(s);
        it = covers.erase(it);
      } else {
        p = dynamic_bitset<>(s).to_ulong();
        init[p] = '1';
        ++it;
      }
    }
    return init;
  }
}

} // namespace

class BLIFLoader : public FudanFPGA::netlist::fileio::Loader {
public:
  BLIFLoader() : Loader("blif") {}

  void load(std::istream &istrm, const std::string &lib);

  void write(std::ostream &os);
};

void BLIFLoader::load(std::istream &istrm, const std::string &lib) {
  blif_loader_impl loader(design(), istrm);
  try {
    loader.load_blif(lib);
  } catch (netlist_error &e) {
    cerr << e.what();
  }
}

static BLIFLoader blif_loader;

void BLIFLoader::write(ostream &os) {
  blif_writer_impl(design(), os).write_blif();
}