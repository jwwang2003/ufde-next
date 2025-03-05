#include "PKApp.h"
#include <boost/lexical_cast.hpp>
#include <deque>
#include <iostream>
#include <netlist.hpp>
#include <string>

namespace PACK {

using namespace std;
using namespace boost;
using namespace VCELL_ATTR;

/************************************************************************/
/*              Carry Chain                                             */
/************************************************************************/

using namespace CHAIN_NAME;

struct ChainCell;

Property<ChainCell *> CHAIN_CELL;

struct ChainCell {
  PKInstance *lut;
  PKInstance *cymux;
  PKInstance *xor_;
  PKInstance *addf;
  int level;
  bool addf_flag;

  ChainCell(VCell *vc)
      : lut(nullptr), cymux(nullptr), xor_(nullptr), addf(nullptr), level(0),
        addf_flag(false) {
    for (const Match::InstPair &inst_pair : vc->inst_pairs()) {
      if (inst_pair.image_inst->down_module()->name() == LUT) {
        lut = inst_pair.image_inst;
        lut->set_property(CHAIN_CELL, this);
      } else if (inst_pair.image_inst->down_module()->name() == CYMUX) {
        cymux = inst_pair.image_inst;
        cymux->set_property(CHAIN_CELL, this);
      } else if (inst_pair.image_inst->down_module()->name() == XOR) {
        xor_ = inst_pair.image_inst;
        xor_->set_property(CHAIN_CELL, this);
      } else if (inst_pair.image_inst->down_module()->name() == ADDF) {
        addf = inst_pair.image_inst;
        addf->set_property(CHAIN_CELL, this);
        addf_flag = true;
      }
    }
  }
};

vector<ChainCell *> first_chain_cells;
vector<PKInstance *> chain_cores;
static int chain_level;
map<ChainCell *, VCell *> ccell_to_vcell;

typedef vector<ChainCell *> SingleChain;
typedef vector<ChainCell *> AllChainCells;

static void convert_vcell_to_chaincell(Packer::VCellList &vcells,
                                       vector<ChainCell *> &chain_cells) {
  for (VCell *vc : vcells) {
    if (vc->type() == VCELL_TYPE::CARRY_CHAIN ||
        vc->type() == VCELL_TYPE::MULTIPLIER) {
      ChainCell *cc = new ChainCell(vc);
      chain_cells.push_back(cc);
      ccell_to_vcell[cc] = vc;
    }
  }
}

static void find_singal_chain_from_cin(PKInstance *seed,
                                       AllChainCells &chain_cells,
                                       SingleChain &chain) {
  if (!is_in(seed, chain_cores))
    return;

  seed->set_used();
  ChainCell *cell = seed->property_value(CHAIN_CELL); //?
  if (!is_in(cell, chain)) {
    chain.push_back(cell);
    cell->level = chain_level--;
  }

  PKInstance *cymux = cell->cymux;

  PKNet *cinet = cymux->find_pin(CYMUX_CI)->net();
  if (cinet != nullptr) {

    PKInstance *ci_cymux;
    PKInstance *ci_seed;
    PKPin *muxcout_pin = cinet->find_pin(CYMUX_O);

    // the cell is in the middle of the chain
    if (muxcout_pin != nullptr &&
        muxcout_pin->instance()->down_module()->name() == CYMUX) {
      ci_cymux = muxcout_pin->instance();
      ci_seed = ci_cymux->property_value(CHAIN_CELL)->lut;
      find_singal_chain_from_cin(ci_seed, chain_cells, chain);
    }
    // first chain cell, its input cell has 2 cases:
    // 1, the cell has no pin named with CYMUX_O
    // 2, the cell luckly has a pin named with CYMUX_O, but it is not CYMUX
    else if (muxcout_pin == nullptr ||
             (muxcout_pin != nullptr &&
              muxcout_pin->instance()->down_module()->name() != CYMUX)) {
      first_chain_cells.push_back(cell);
      return;
    }
  }
}

static void find_single_chain_from_addf_cin(PKInstance *seed,
                                            AllChainCells &chain_cells,
                                            SingleChain &chain) {
  if (!is_in(seed, chain_cores))
    return;

  seed->set_used();
  ChainCell *cell = seed->property_value(CHAIN_CELL); //?
  if (!is_in(cell, chain)) {
    chain.push_back(cell);
    cell->level = chain_level--;
    // cout<<cell->level<<endl;
  }

  PKInstance *addf = cell->addf;

  PKNet *cinet = addf->find_pin(ADDF_CI)->net();
  if (cinet != nullptr) {

    PKInstance *ci_addf;
    PKInstance *ci_seed;
    PKPin *addfcout_pin = cinet->find_pin(ADDF_CO);

    // the cell is in the middle of the chain
    if (addfcout_pin != nullptr &&
        addfcout_pin->instance()->down_module()->name() == ADDF) {
      ci_addf = addfcout_pin->instance();
      ci_seed = ci_addf->property_value(CHAIN_CELL)->addf;
      if (ci_seed->is_used()) {
        first_chain_cells.push_back(cell);
        return;
      }
      find_single_chain_from_addf_cin(ci_seed, chain_cells, chain);
    }
    // first chain cell, its input cell has 2 cases:
    // 1, the cell has no pin named with CYMUX_O
    // 2, the cell luckly has a pin named with CYMUX_O, but it is not CYMUX
    else if (addfcout_pin == nullptr ||
             (addfcout_pin != nullptr &&
              addfcout_pin->instance()->down_module()->name() != ADDF)) {
      first_chain_cells.push_back(cell);
      return;
    }
  }
}

static void find_singal_chain_from_cout(PKInstance *seed,
                                        AllChainCells &chain_cells,
                                        SingleChain &chain) {
  if (!is_in(seed, chain_cores))
    return;

  seed->set_used();
  ChainCell *cell = seed->property_value(CHAIN_CELL);
  if (!is_in(cell, chain)) {
    chain.push_back(cell);
    cell->level = chain_level++;
  }

  PKInstance *cymux = cell->cymux;

  PKNet *conet = cymux->find_pin(CYMUX_O)->net();
  if (conet != nullptr) {

    PKInstance *co_cymux;
    PKInstance *co_seed;
    PKInstance *co_xor;
    PKPin *muxcin_pin = conet->find_pin(CYMUX_CI);
    PKPin *xorcin_pin = conet->find_pin(XOR_CI);

    // the cell is in the middle of the chain
    if (muxcin_pin != nullptr &&
        muxcin_pin->instance()->down_module()->name() == CYMUX) {
      co_cymux = muxcin_pin->instance();
      co_seed = co_cymux->property_value(CHAIN_CELL)->lut;
      find_singal_chain_from_cout(co_seed, chain_cells, chain);
    }
    // this is the second last carry chain cell
    // and the last chain cell has no cymux(only lut+xor)
    else if (muxcin_pin == nullptr && xorcin_pin != nullptr &&
             xorcin_pin->instance()->down_module()->name() == XOR) {
      co_xor = xorcin_pin->instance();
      co_seed = co_xor->property_value(CHAIN_CELL)->lut;
      co_seed->set_used();
      ChainCell *next_cell = co_xor->property_value(CHAIN_CELL);
      chain.push_back(next_cell);
      next_cell->level = chain_level++;
    }
    // this is the last carry chain cell with cymux
    else if (muxcin_pin == nullptr ||
             (muxcin_pin != nullptr &&
              muxcin_pin->instance()->down_module()->name() != CYMUX)) {
      ;
    }
    return;
  }
}
static void find_single_chain_from_addf_cout(PKInstance *seed,
                                             AllChainCells &chain_cells,
                                             SingleChain &chain) {
  if (!is_in(seed, chain_cores))
    return;

  seed->set_used();
  ChainCell *cell = seed->property_value(CHAIN_CELL);
  if (!is_in(cell, chain)) {
    chain.push_back(cell);
    cell->level = chain_level++;
    // cout<<cell->level<<endl;
  }

  PKInstance *addf = cell->addf;

  PKNet *conet = addf->find_pin(ADDF_CO)->net();
  if (conet != nullptr) {

    PKInstance *co_addf;
    PKInstance *co_seed;
    PKInstance *co_xor;
    PKPin *addfcin_pin = conet->find_pin(ADDF_CI);
    // PKPin* xorcin_pin = conet->find_pin(XOR_CI);

    // the cell is in the middle of the chain
    if (addfcin_pin != nullptr &&
        addfcin_pin->instance()->down_module()->name() == ADDF) {
      co_addf = addfcin_pin->instance();
      co_seed = co_addf->property_value(CHAIN_CELL)->addf;
      if (co_seed->is_used())
        return;
      find_single_chain_from_addf_cout(co_seed, chain_cells, chain);
    }
    // this is the second last carry chain cell
    // and the last chain cell has no cymux(only lut+xor)
    // else if(muxcin_pin == nullptr && xorcin_pin != nullptr &&
    // xorcin_pin->owner().instof().name() == XOR){ 	co_xor =
    //&xorcin_pin->owner(); 	co_seed =
    // co_xor->property_value<ChainCell*>(CHAIN_CELL)->lut;
    // co_seed->set_used(); 	ChainCell* next_cell =
    //co_xor->property_value<ChainCell*>(CHAIN_CELL);
    //	chain.push_back(next_cell);
    //	next_cell->level = chain_level++;
    //}
    // this is the last carry chain cell with cymux
    else if (addfcin_pin == nullptr ||
             (addfcin_pin != nullptr &&
              addfcin_pin->instance()->down_module()->name() != ADDF)) {
      ;
    }
    return;
  }
}

static void find_singal_chain(PKInstance *seed, AllChainCells &chain_cells,
                              SingleChain &chain) {
  chain_level = 0;
  find_singal_chain_from_cin(seed, chain_cells, chain);
  chain_level = 1;
  find_singal_chain_from_cout(seed, chain_cells, chain);
}
static void find_single_addf_chain(PKInstance *seed, AllChainCells &chain_cells,
                                   SingleChain &chain) {
  chain_level = 0;
  find_single_chain_from_addf_cin(seed, chain_cells, chain);
  chain_level = 1;
  find_single_chain_from_addf_cout(seed, chain_cells, chain);
}

static void sort_carrt_chain(vector<SingleChain> &chains) {
  for (SingleChain &chain : chains) {
    deque<ChainCell *> dq_chain;
    for (ChainCell *cell : chain) {
      if (dq_chain.empty())
        dq_chain.push_back(cell);
      else {
        if (cell->level < dq_chain.front()->level)
          dq_chain.push_front(cell);
        else if (cell->level > dq_chain.back()->level)
          dq_chain.push_back(cell);
        else
          ASSERT(0, "There are two chain cells in the same chain with the same "
                    "level.");
      }
    }
    chain.clear();
    chain_level = 0;
    for (ChainCell *cell : dq_chain) {
      chain.push_back(cell);
      cell->level = chain_level++;
    }
  }
}

static void group_carry_chain(AllChainCells &chain_cells,
                              vector<SingleChain> &chains) {
  PKInstance *seed; // Lut or ADDF

  for (ChainCell *cell : chain_cells) {
    if (cell->addf_flag) {
      cell->addf->clear_used();
      chain_cores.push_back(cell->addf);
    } else {
      cell->lut->clear_used();
      chain_cores.push_back(cell->lut);
    }
  }

  for (ChainCell *cell : chain_cells) {
    // make sure in ChainCell for the seed, there is a CYMUX
    if (cell->addf_flag) {
      if (!cell->addf->is_used()) {
        seed = cell->addf;
        seed->set_used();
        SingleChain chain;
        find_single_addf_chain(seed, chain_cells, chain);
        chains.push_back(chain);
      }
    } else {
      if (cell->cymux == nullptr)
        continue;
      if (!cell->lut->is_used()) {
        seed = cell->lut;
        seed->set_used();
        SingleChain chain;
        find_singal_chain(seed, chain_cells, chain);
        chains.push_back(chain);
      }
    }
  }

  // check if all carry chain cell is grouped
  for (ChainCell *cell : chain_cells) {
    if (cell->addf_flag) {
      ASSERT(cell->addf->is_used(), "There is addf in chain cell not grouped.");
    } else {
      ASSERT(cell->lut->is_used(), "There is lut in chain cell not grouped.");
    }
  }
  // sort carry chain
  sort_carrt_chain(chains);

  for (ChainCell *cell : chain_cells) {
    if (cell->addf_flag) {
      cell->addf->clear_used();
    } else {
      cell->lut->clear_used();
    }
  }
}

void Packer::pack_carry_chain() {
  vector<ChainCell *> chain_cells;
  convert_vcell_to_chaincell(uncluster_vcells_, chain_cells);
  Config &CYINIT = create_config(CFG_NAME::SLICE, "CYINIT");
  Config &YBMUX = create_config(CFG_NAME::SLICE, "YBMUX");
  Config *XBUSED = 0;
  Config *BXMUX = 0;
  Config *XBMUX = 0;
  Config *BXINV = 0;
  Config *YBUSED = 0;
  if (chip_type_ == CHIP_TYPE_NAME::FDP3) {
    XBUSED = &create_config(CFG_NAME::SLICE, "XBUSED");
    BXMUX = &create_config(CFG_NAME::SLICE, "BXMUX");
  }
  if (chip_type_ == CHIP_TYPE_NAME::FDP4) { // add by zzzhang. for fdp4
    XBMUX = &create_config(CFG_NAME::SLICE, "XBMUX");
    BXINV = &create_config(CFG_NAME::SLICE, "BXINV");
    YBUSED = &create_config(CFG_NAME::SLICE, "YBUSED");
  }
  Property<string> &H_SET = create_property<string>(INSTANCE, "h_set");
  Property<string> &SET_TYPE = create_property<string>(INSTANCE, "set_type");
  Property<Point> &RLOC = create_property<Point>(INSTANCE, "rloc");

  vector<SingleChain> chains;
  group_carry_chain(chain_cells, chains);

  typedef pair<VCell *, VCell *> CellPair;
  vector<vector<PKInstance *>> slice_chains; // slice chains

  // ��������λ�����slice
  for (SingleChain chain : chains) {
#ifdef _DEBUG
    cout << "=====================================================" << endl;
    cout << "CARRY CHAIN INFO: Start packing a carry chain" << endl;
#endif
    vector<CellPair> cc_slices;
    for (size_t i = 0; i < chain.size(); i += 2) {
      VCell *f = ccell_to_vcell[chain[i]];
      VCell *s = (i + 1 == chain.size()) ? nullptr : ccell_to_vcell[chain[i + 1]];
      cc_slices.push_back(make_pair(f, s));
    }

    vector<PKInstance *> aslice_chain;
    for (CellPair &cp : cc_slices) {
      tf_.set_sfull_flag();
      tf_.transform(cp.first); // �ȴ�λ��LUT F�Ľ�λ��

#ifdef _DEBUG
      cout << "CARRY CHAIN INFO: [F] This level is packed into Slice F" << endl;
#endif
      PKInstance *slice = tf_.slice_inst();
      aslice_chain.push_back(slice);

      PKPin *fcout = static_cast<PKPin *>(slice->find_pin(PIN_NAME::COUT));
      PKNet *fcout_net = fcout->net();

      if (fcout_net != nullptr) {
        Net::pins_type f_sink_pin_range_ = fcout_net->sink_pins();

#ifdef _DEBUG
        for (Net::pin_iter p_it = f_sink_pin_range_.begin();
             p_it != f_sink_pin_range_.end(); ++p_it)
          cout << "\tsink pins:" << p_it->owner() << "->" << p_it->name()
               << endl;
#endif
      }
      // ��XB�������
      // ���1�����һ����λ����cout��XB��ȥ��
      // ���2����Ȼ�������һ�������Ǳ��������Ҫ�õ���λ�����ݣ�Ҳ��XB��ȥ

      // ���1��
      if (fcout_net != nullptr && cp.second == nullptr) {

        /*cout<<"--------------------------------------------"<<endl;
        cout<<"net name: "<<fcout_net->name()<<endl;
        cout<<"sink pins:"<<fcout_net->sink_pins()<<endl;
        cout<<"number of sink pins =\t"<<fcout_net->sink_pins().size()<<endl;*/

        PKPin *xb = static_cast<PKPin *>(slice->find_pin(PIN_NAME::XB));
        ASSERT(xb->net() == nullptr, "XB has been occupyied.-1");
        xb->connect(fcout_net);
        xb->set_used();
        if (chip_type_ == CHIP_TYPE_NAME::FDP3)
          slice->set_property(*XBUSED, "0");
        else if (chip_type_ == CHIP_TYPE_NAME::FDP4)
          slice->set_property(*XBMUX, "1");
        else
          ASSERT(0, "Chip type error. site:1");

#ifdef _DEBUG
        cout << "CARRY CHAIN INFO: [F] This is the last level of the carry "
                "chain. Carry out by XBUSED/XBMUX."
             << endl;
        cout << "\tnet name:" << fcout_net->name() << endl;
#endif
      }

      // ���2��

      else if (fcout_net != nullptr && fcout_net->sink_pins().size() > 1) {
        PKPin *xb = static_cast<PKPin *>(slice->find_pin(PIN_NAME::XB));
        ASSERT(xb->net() == nullptr, "XB has been occupyied.-2");
        xb->connect(fcout_net);
        xb->set_used();
        if (chip_type_ == CHIP_TYPE_NAME::FDP3)
          slice->set_property(*XBUSED, "0");
        else if (chip_type_ == CHIP_TYPE_NAME::FDP4)
          slice->set_property(*XBMUX, "1");
        else
          ASSERT(0, "Chip type error. site:2");

#ifdef _DEBUG
        cout << "CARRY CHAIN INFO: [F] This level has multi-sink pin.  Carry "
                "out by XBUSED/XBMUX."
             << endl;
        cout << "\tnet name:" << fcout_net->name() << endl;
#endif
      }

      // ��Ϊ��ƥ��������cout������Ҫ�رգ����ҽ�cout���Ƴ��������ӵ�����
      //
      if (fcout_net != nullptr) {
        fcout->disconnect();
        fcout->clear_used();
        Config &COUTUSED = create_config("SLICE", "COUTUSED");
        COUTUSED.remove(slice);
      }

      set_vcell_clusterd(cp.first);
      tf_.clear_sfull_flag();
      if (cp.second != nullptr) {
#ifdef _DEBUG
        cout << "CARRY CHAIN INFO: [G] This level is packed into Slice G"
             << endl;
#endif
        tf_.transform(cp.second);
        fcout_net = fcout->net(); // ���cout�����ӵ�����

        if (fcout_net != nullptr) {
          Net::pins_type g_sink_pin_range_ = fcout_net->sink_pins();

#ifdef _DEBUG
          for (Net::pin_iter p_it = g_sink_pin_range_.begin();
               p_it != g_sink_pin_range_.end(); ++p_it) {
            cout << "\tsink pins:" << p_it->owner() << "->" << p_it->name()
                 << endl;
          }
#endif
        }
        // ��λ�����һ���Ľ�λ�����Ӧ�ô�cout��ȥ��ise�������Լ��Ĳ��߶����Զ���cout�����������ӵ�YB����ȥ����һ������Ҫpacking����������
        // �����ǵĲ��ֲ�����Ҫ�����һ����YBMUX��

        // �������if���������������͸����
        // ��һ�δ��봦��cout�����ӵ���������
        //
        // ������������������������3�����
        // 1-srcpin�еͼ���λ����cout��sinkpin�и߼���λ����cin��һ������slice��iob������pin
        // 2-srcpin�еͼ���λ����cout��sinkpin�и߼���λ����cin���������slice��iob������pin
        // 3-srcpin�еͼ���λ����cout��sinkpin�ж������slice��iob������pin
        if (fcout_net != nullptr && fcout_net->sink_pins().size() > 1) {

#ifdef _DEBUG
          cout << "CARRY CHAIN INFO: [G] Multi-sink pins in CARRYCHAIN" << endl;
          cout << "\tnet name:" << fcout_net->name() << endl;
#endif

          /*cout<<"--------------------------------------------"<<endl;
          cout<<"net name: "<<fcout_net->name()<<endl;

          cout<<"sink pins:"<<fcout_net->sink_pins()<<endl;
          cout<<"number of sink pins
          =\t"<<fcout_net->sink_pins().size()<<endl;*/

          Net::pins_type sink_pin_range_ = fcout_net->sink_pins();
          bool has_cin = false;

          for (Net::pin_iter p_it = sink_pin_range_.begin();
               p_it != sink_pin_range_.end(); ++p_it) {
            if (p_it->name() == CHAIN_NAME::ADDF_CI) {
              has_cin = true;
              break;
            }
          }

          /*for (Net::pin_iter
          p_it=sink_pin_range_.begin();p_it!=sink_pin_range_.end();++p_it){
                  cout<<"\tsink pins:"<<p_it->owner()<<"->"<<p_it->name()<<endl;
          }*/

          if (has_cin) {
// �ȴ�һ���µ�net��boy_net_name_
#ifdef _DEBUG
            cout << "CARRY CHAIN INFO: [G] Divide carry chain net to two nets"
                 << endl;
#endif

            Module *net_owner_ = fcout_net->module();
            const string boy_net_name_ = fcout_net->name() + "_boy_net";
            Net *boy_net_ = net_owner_->create_net(
                boy_net_name_, NORMAL); // Net& create_net(const string&
                                        // net_name, NetType type = NORMAL)

            vector<Pin *> pin_ptr_vec;
            for (Net::pin_iter p_it = sink_pin_range_.begin();
                 p_it != sink_pin_range_.end(); ++p_it) {
              cout << "name=" << p_it->name() << endl;
              if (p_it->name() !=
                  CHAIN_NAME::
                      ADDF_CI) { // �ѳ��˸߼���λ����cin�������ȫ��pin�ҳ���,�Ž�pin_ptr_vec
                Pin *new_pin_ptr_ = (*p_it);
                pin_ptr_vec.push_back(new_pin_ptr_);
              }
            }

            // ������pin_ptr_vec���pinȫ���������ӵ��µ�������ȥ
            for (vector<Pin *>::iterator it = pin_ptr_vec.begin();
                 it != pin_ptr_vec.end(); ++it) {
              (*it)->reconnect(boy_net_);
              cout << "new pin = " << (*it)->net()->name() << endl;
            }

            PKPin *yb = static_cast<PKPin *>(slice->find_pin(PIN_NAME::YB));
            ASSERT(yb->net() == nullptr, "YB has been occupyied.");
            yb->connect(boy_net_); // ��ybҲhookup����������
            yb->set_used();
            slice->set_property(YBMUX, "1");
            if (chip_type_ == CHIP_TYPE_NAME::FDP4)
              slice->set_property(*YBUSED, "0");

#ifdef _DEBUG
            cout << "CARRY CHAIN INFO: [G] Use YBMUX for the other outlet"
                 << endl;
            cout << "\t1st net name:" << fcout_net->name() << endl;
            cout << "\t2nd net name:" << boy_net_->name() << endl;
#endif
          }
        }
        set_vcell_clusterd(cp.second);
      }
      // process carry chain's first cell
      // if the cell's cin is connected, reconnect it to BX
      // if the cell's cin is not connected, that means it is connected to
      // constant originally
      for (ChainCell *ccell : first_chain_cells) {
        if (cp.first == ccell_to_vcell[ccell]) {
          PKPin *fcin = static_cast<PKPin *>(slice->find_pin(PIN_NAME::CIN));
          PKPin *bx = static_cast<PKPin *>(slice->find_pin(PIN_NAME::BX));
          PKNet *fcin_net = fcin->net();
          if (fcin_net != nullptr && bx->net() == nullptr) {
            fcin->disconnect();
            fcin->clear_used();
            bx->connect(fcin_net);
            bx->set_used();
            if (chip_type_ == CHIP_TYPE_NAME::FDP3)
              slice->set_property(*BXMUX, "BX");
            else if (chip_type_ == CHIP_TYPE_NAME::FDP4)
              slice->set_property(*BXINV, "BX");
            else
              ASSERT(0, "Chip type error. site:4");

            slice->set_property(CYINIT, "BX");
          }
        }
      }
    }
    slice_chains.push_back(aslice_chain);
  }
  // add chain's property:
  // h_set: first chain slice's name
  // rloc:  chain's coordinate, (x,0,0) for the last chain slice, x=0
  // set_type: carry_chain
  for (vector<PKInstance *> &vinst : slice_chains) {
    string h_set = vinst[0]->name();
    string rloc;
    int i = (int)vinst.size() - 1;
    for (PKInstance *inst : vinst) {
      inst->set_property(H_SET, h_set);
      inst->set_property(RLOC, Point(i--, 0, 0));
      inst->set_property(SET_TYPE, "carry_chain");
    }
  }
}

/************************************************************************/
/*             Multiplier                                               */
/************************************************************************/

void Packer::pack_multiplier() {
  // pack_carry_chain();
  // add by zzzhang 2011/3/2
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::MULTIPLIER) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      set_vcell_clusterd(vc);
    }
  }
}

/************************************************************************/
/*              Lut5                                                    */
/************************************************************************/

void Packer::pack_lut5() {
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::LUT5) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      set_vcell_clusterd(vc);
    }
  }
}

/************************************************************************/
/*              Lut6                                                    */
/************************************************************************/

void Packer::pack_lut6() {
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::LUT6) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      set_vcell_clusterd(vc);
    }
  }
}

/************************************************************************/
/*        Block RAM                                                     */
/************************************************************************/

void Packer::pack_bram() {
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::BRAM) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      set_vcell_clusterd(vc);
    }
  }
}

/************************************************************************/
/*        Distributed RAM                                               */
/************************************************************************/

void Packer::pack_dram() {
  list<VCell *> uncluster_loop(uncluster_vcells_);
  for (VCell *vc : uncluster_loop) {
    if (vc->type() == VCELL_TYPE::DRAM) {
      tf_.set_sfull_flag();
      tf_.transform(vc);
      set_vcell_clusterd(vc);
    }
  }
}

} // namespace PACK