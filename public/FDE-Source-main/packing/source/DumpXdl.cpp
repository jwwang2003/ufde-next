#include "PKApp.h"
#include "Packer.h"
#include "zfstream.h"

namespace PACK {

using namespace std;

static void inst_cfg(Instance *inst, ostream &xdl_file,
                     ConfigInfo &config_info) {
  string inst_type = inst->module_type();

  xdl_file << "inst \"" << inst->name() << "\" \"" << inst_type
           << "\",unplaced bonded ,\n"
           << "\tcfg \" \n";

  for (PKConfig *cfg : config_info.get_configs(inst_type))
    cfg->xdl_out(xdl_file, inst);

  xdl_file << "\t\t\"\n;\n" << endl;
}

// �˺���ӦmakejieҪ��������ӵ�pad������
// ���������������������design_timesim.v�˿����ƻ��ң��޷�����
static void net_cfg(Net *all_net, ostream &xdl_file) {
  xdl_file << "\nnet \"" << all_net->name() << "\" ,";
  for (Pin *pin : all_net->pins()) {
    if (pin->is_mpin() || pin->name() == "PAD")
      continue;
    if (pin->is_source())
      xdl_file << "\n  outpin \"" << pin->instance()->name().c_str() << "\" "
               << pin->name() << " ,";
    else if (pin->is_sink())
      xdl_file << "\n  inpin \"" << pin->instance()->name().c_str() << "\" "
               << pin->name() << " ,";
  }
  xdl_file << "\n  ;";
}

void Packer::write_xdl(Design *design, bool fencry) {
  FDU_LOG(DEBUG) << "Start to generate XDL file.";

  map<string, int> cell_count;

  const string &xdl_fname = PKApp::instance().xdl_fname();
  ostringstream xdl_file;

  // xdl_file.open(xdl_fname.c_str());
  xdl_file << "# =======================================================\n";
  if (chip_type_ == CHIP_TYPE_NAME::FDP3) {
    xdl_file << "design "
             << "\"" << design->name() << "\""
             << " xc2s300epq208-6 v3.1;";
  } else if (chip_type_ == CHIP_TYPE_NAME::FDP4) {
    xdl_file << "design "
             << "\"" << design->name() << "\""
             << " xc2v80fg256-6 v3.1;";
  } else
    ASSERT(0, "Chip type error. site:0");
  xdl_file << "\n#=======================================================\n";
  // iob,slice,blockram

  for (Instance *inst : design->top_module()->instances()) {
    inst_cfg(inst, xdl_file, config_info_);
    cell_count[inst->module_type()]++;
    if (inst->module_type() == CFG_NAME::IOB) {
      for (PKConfig *cfg : config_info_.get_configs(CFG_NAME::IOB)) {
        if (inst->property_exist(cfg->cos_cfg()) &&
            inst->property_value(cfg->cos_cfg()) != CFG_NAME::DEF_VALUE) {
          cell_count[CFG_NAME::DANGLEIOB]++;
          break;
        }
      }
    }
  }

  for (Net *net : design->top_module()->nets())
    net_cfg(net, xdl_file);

  if (fencry) {
    zofstream zofs(xdl_fname.c_str());
    zofs << xdl_file.str();
  } else {
    ofstream ofs(xdl_fname.c_str());
    ofs << xdl_file.str();
    ofs.close();
  }

  const string &rpt_fname = PKApp::instance().rpt_fname();
  ofstream rpt_file;
  rpt_file.open(rpt_fname.c_str());
  rpt_file << design->name() << "\n";
  rpt_file << "IOB number="
           << cell_count[CFG_NAME::IOB] - cell_count[CFG_NAME::DANGLEIOB]
           << "\n";
  rpt_file << "SLICE number=" << cell_count[CFG_NAME::SLICE] << "\n";
  rpt_file << "BLOCK RAM number=" << cell_count[CFG_NAME::BLOCKRAM] << "\n";
  rpt_file << "GCLK number=" << cell_count[CFG_NAME::GCLK] << "\n";
  rpt_file << "GCLKIOB number=" << cell_count[CFG_NAME::GCLKIOB] << "\n";
  rpt_file << "BUFGMUX number=" << cell_count[CFG_NAME::BUFGMUX] << "\n";
  rpt_file << "DANGLE IOB number=" << cell_count[CFG_NAME::DANGLEIOB] << "\n";
  rpt_file << "TBUF number=" << cell_count[CFG_NAME::TBUF] << "\n";
  rpt_file.close();

  if (cell_count[CFG_NAME::DANGLEIOB] > 0)
    FDU_LOG(DEBUG) << "TIPS: Design has " << cell_count[CFG_NAME::DANGLEIOB]
                   << " DANGLE IOB!";
  FDU_LOG(DEBUG) << "Generate XDL file successfully.";
}

} // namespace PACK