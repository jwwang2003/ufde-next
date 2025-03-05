#include "plc_app.h"
#include "arch/archlib.hpp"
#include "plc_utils.h"
#include "xmlutils.h"

#include <ctime>

#include <boost/algorithm/string/case_conv.hpp>

// #define EXCEPTION_HANDLE

namespace FDU {
namespace Place {

using namespace std;
using namespace ARCH;
using namespace COS;

/************************************************************************/
/*	���ܣ������û����ݽ�������
 *	������argc�� ���������� argv �� �����б�
 *	����ֵ�� void
 */
/************************************************************************/
void PlaceApp::parse_command(int argc, char *argv[]) {
  INFO(CONSOLE::COPY_RIGHT);

  INFO(CONSOLE::PROGRESS % "0" % "parsing commands");
  _args.try_parse(argc, argv);
}

/************************************************************************/
/*	���ܣ����벼������ļ�
 *	������	void
 *	����ֵ��void
 */
/************************************************************************/
void PlaceApp::load_files() {
  // ����arch�ļ�
  INFO(CONSOLE::PROGRESS % "10" %
       ("loading arch library \"" + _args._arch_lib + "\""));
  try {
    FPGADesign::instance()->loadArchLib(_args._arch_lib, new ArchFactory);
  } catch (exception &e) {
    ERR(CONSOLE::FILE_ERROR % "can NOT load arch library.");
    exit(EXIT_FAILURE);
  }
  // ���������ļ�
  INFO(CONSOLE::PROGRESS % "20" %
       ("loading netlist \"" + _args._input_nl + "\""));
  try {
    CosFactory::set_factory(new PLCFactory());
    _design.load("xml", _args._input_nl);
  } catch (exception &e) {
    ERR(CONSOLE::FILE_ERROR % "can NOT load netlist.");
    exit(EXIT_FAILURE);
  }
  // ����Լ���ļ�
  if (!_args._plc_csts.empty()) {
    INFO(CONSOLE::PROGRESS % "30" %
         ("loading constraint \"" + _args._plc_csts + "\""));
    try {
      _design.load("constraint", _args._plc_csts);
    } catch (exception &e) {
      ERR(CONSOLE::FILE_ERROR % "can NOT load constraint.");
      exit(EXIT_FAILURE);
    }
  } else {
    INFO(CONSOLE::PROGRESS % "30" % "no constraint loaded");
  }
// ����template cell library����ISE�����ã�
#ifdef CONST_GENERATE
  if (!_args._library.empty()) {
    INFO(CONSOLE::PROGRESS % "35" %
         ("loading template cell library \"" + _args._library + "\""));
    try {
      _design.load(_args._encrypt ? "xml*" : "xml", _args._library);
    } catch (exception &e) {
      ERR(CONSOLE::FILE_ERROR % "can NOT load template cell library.");
      exit(EXIT_FAILURE);
    }
  } else {
    INFO(CONSOLE::PROGRESS % "35" % "no constraint loaded");
  }
#endif

  // ��ʾEffort Level
  INFO(CONSOLE::EFFORT_LEVEL % _args._effort_level);
  // �ж��Ƿ���ʱ����������������ʱ��Ϣ
  if (_args._mode == PLACER::TIMING_DRIVEN) {
    INFO(CONSOLE::PLC_MODE % "Timing Driven");
    INFO(CONSOLE::PROGRESS % "40" %
         ("loading delay library \"" + _args._delay_lib + "\""));
    try {
      _design.load("delay", _args._delay_lib);
    } catch (exception &e) {
      ERR(CONSOLE::FILE_ERROR % "can NOT load delay library.");
      exit(EXIT_FAILURE);
    }
  } else if (_args._mode == PLACER::BOUNDING_BOX) {
    INFO(CONSOLE::PLC_MODE % "Bounding Box");
  }
}

void PlaceApp::export_jtag_csts() {
  //		static boost::format CST  ("|%-30s|%-10s|%-5d|\n");
  static boost::format CST("%1%[\"%2%\"] = %3%\n");
  static boost::format BEGIN("## begin: %1% ##\n");
  static boost::format END("## end ##\n");
  static const string INPUTS = "inputs";
  static const string OUTPUTS = "outputs";
  static const string IPIN = "I";
  static const string OPIN = "O";
  static const string OPIN1 = "O1";
  static const string OPIN2 = "O2";

  if (_args._jtag_csts.empty())
    return;

  ofstream ofs(_args._jtag_csts.c_str());
  ASSERT(ofs.is_open(),
         (CONSOLE::FILE_ERROR % "can NOT write jtag constraints."));

  FPGADesign *arch_lib = FPGADesign::instance();

  ofs << BEGIN % _design.name();
  for (const Instance *inst : _design.top_module()->instances()) {
    bool echo = false;
    string dir("");
    string cell_type = inst->module_type();
    if (cell_type == CELL::IO) {
      echo = true;
      const Pin *pin = nullptr;
      if ((pin = inst->pins().find(IPIN)) && pin->net())
        dir = INPUTS;
      else if ((pin = inst->pins().find(OPIN)) && pin->net() ||
               (pin = inst->pins().find(OPIN1)) && pin->net() ||
               (pin = inst->pins().find(OPIN2)) && pin->net())
        dir = OUTPUTS;
      else {
        //					throw
        // place_error(CONSOLE::PLC_ERROR % (inst.name() + ": INVALID IOB for
        // jtag constraints."));
        echo = false;
        INFO(CONSOLE::PLC_WARNING %
             (inst->name() + ": IGNORE dangling JTAG IOB."));
      }
    } else if (cell_type == CELL::GCLKIOB) {
      echo = true;
      dir = INPUTS;
    }

    if (echo) {
      Property<Point> &positions =
          create_property<Point>(COS::INSTANCE, INSTANCE::POSITION);
      string pad_name =
          arch_lib->find_pad_by_position(inst->property_value(positions));
      ofs << CST % dir // jtag need to have lower case name
                 % boost::algorithm::to_lower_copy(inst->name()) % pad_name;
      //%pad_name.substr(1);
    }
  }
  ofs << END;
}

/************************************************************************/
/*	���ܣ� ���֣�����_placer:SAPlacer���в���
 *	������void
 *	����ֵ�� void
 */
/************************************************************************/
void PlaceApp::try_process() {
#ifdef EXCEPTION_HANDLE
  try {
#endif
    time_t t_start, t_end;
    time(&t_start);

    if (_args._update_rand_seed)
      srand(t_start);

    // ��������ļ�
    load_files();
    // ���ò���ģʽ
    _placer.set_place_mode(_args._mode);
    _placer.set_effort_level(_args._effort_level);
    // ��ʼ����
    _placer.try_place(_args);
    // ������ֽ��
    INFO(CONSOLE::PROGRESS % "90" % "begin to write placed netlist");
    _design.save("xml", _args._output_nl, _args._encrypt);
    export_jtag_csts();

    time(&t_end);
    INFO(CONSOLE::FINISH % difftime(t_end, t_start));

#ifdef EXCEPTION_HANDLE
  }
  // 		catch(place_error& e){
  // 			ERR(e.what());
  // 			exit(EXIT_FAILURE);
  // 		}
  // 		catch(Exception& e){
  // 			ERR(e.what());
  // 			exit(EXIT_FAILURE);
  // 		}
  catch (exception &e) {
    ERR(e.what());
    exit(EXIT_FAILURE);
  } catch (...) {
    ERR("[PLC_ERROR] unknown error occurred.");
    exit(EXIT_FAILURE);
  }
#endif
}

} // namespace Place
} // namespace FDU