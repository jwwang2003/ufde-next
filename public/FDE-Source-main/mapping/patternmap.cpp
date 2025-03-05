#include "log.h"
#include "mapping.hpp"

namespace {
using namespace std;
using namespace COS;

void addBuf(Design *pDesign) {
  Library *cell_lib = pDesign->find_library("cell_lib");
  Module *top = pDesign->top_module();
  Module *CKBUF = cell_lib->find_module("CLKBUF");
  Module *IBUF = cell_lib->find_module("IBUF");
  Module *OBUF = cell_lib->find_module("OBUF");
  Module *IOBUF = cell_lib->find_module("IOBUF");
  Module *IPAD = cell_lib->find_module("IPAD");
  Module *OPAD = cell_lib->find_module("OPAD");

  vector<Port *> Pi, Po, Pio;
  for (Port *port : top->ports()) {
    switch (port->dir()) {
    case INPUT:
      Pi.push_back(port);
      break;
    case OUTPUT:
      Po.push_back(port);
      break;
    case INOUT:
      Pio.push_back(port);
      break;
    default:
      break;
    }
  }

  for (Port *pi : Pi) {
    ASSERTS(pi->dir() == INPUT);
    for (Pin *pin : pi->mpins()) {
      if (!pin->is_connected()) { // �������Port����
        FDU_LOG(WARN) << "The Pi{" << pin->name() << ") has no fanout";
        Instance *ibuf = top->create_instance("Buf-pad-" + pin->name(), IBUF);
        Net *net = top->create_net(pin->name());
        ibuf->find_pin("I")->connect(net);
        pin->connect(net);
      } else {
        bool isClk = false;
        for (Pin *sink_pin : pin->net()->sink_pins()) {
          if (sink_pin->type() == CLOCK || sink_pin->name() == "CK" ||
              sink_pin->name() == "CKN" || sink_pin->name() == "CLK") {
            isClk = true;
            break;
          }
        }
        if (isClk) { // ����clk_buf��clk_ibuf
          Instance *clk_buf =
              top->create_instance("Buf-pad-" + pin->name(), CKBUF);
          Instance *clk_ibuf =
              top->create_instance("IBuf-clkpad-" + pin->name(), CKBUF);
          clk_ibuf->find_pin("O")->connect(pin->net());
          pin->net()->rename("net_" + clk_ibuf->name());
          Net *net1 = top->create_net(pin->name());
          pin->reconnect(net1);
          clk_buf->find_pin("I")->connect(net1);
          Net *net2 = top->create_net("net_Buf-pad-" + pin->name());
          clk_buf->find_pin("O")->connect(net2);
          clk_ibuf->find_pin("I")->connect(net2);
        } else { // ����input����port
          Instance *ibuf = top->create_instance("Buf-pad-" + pin->name(), IBUF);
          ibuf->find_pin("O")->connect(pin->net());
          pin->net()->rename("net_Buf-pad-" + pin->name());
          Net *net = top->create_net(pin->name());
          ibuf->find_pin("I")->connect(net);
          pin->reconnect(net);
        }
      }
    }
  }
  // ����ipad
  for (Port *pi : Pi) {
    ASSERTS(pi->dir() == INPUT);
    for (Pin *pin : pi->mpins()) {
      Instance *ipad = top->create_instance(pin->name() + "_ipad", IPAD);
      ipad->find_pin("PAD")->connect(pin->net());
    }
  }

  for (Port *po : Po) {
    ASSERTS(po->dir() == OUTPUT);
    for (Pin *pin : po->mpins()) {
      Instance *obuf = top->create_instance("Buf-pad-" + pin->name(), OBUF);
      obuf->find_pin("I")->connect(pin->net());
      pin->net()->rename("net_" + obuf->name());
      Net *net = top->create_net(pin->name());
      obuf->find_pin("O")->connect(net);
      pin->reconnect(net);
    }
  }
  // ����opad
  for (Port *po : Po) {
    ASSERTS(po->dir() == OUTPUT);
    for (Pin *pin : po->mpins()) {
      Instance *opad = top->create_instance(pin->name() + "_opad", OPAD);
      opad->find_pin("PAD")->connect(pin->net());
    }
  }
#if 0
	Pin* pSoucePin = nullptr, * pTempPin = nullptr;
	for (Port* Pio : top->ports()) {
		if (Pio->dir() != INOUT) continue;

		Instance* Buf = top->create_instance("Buf-pad-" + Pio->name(), IBUF);

		for (Pin* pTempPin : Pio->mpin()->net()->pins()) {
			if (pTempPin->dir()==OUTPUT) {
				pSoucePin = pTempPin;
			}
		}
		for (Pin* pTempPin : Pio->mpin()->net()->pins()) {
			if (pTempPin->dir() == INPUT) {
				pTempPin->disconnect();
				Net* net = top->create_net("");// !!
				Buf->find_pin("O")->connect(net);
				pTempPin->connect(net);
			}
		}
		Net* net = top->create_net("");// !!
		pSoucePin->connect(net);
		Buf->find_pin("I")->connect(net);
		Buf->find_pin("I")->connect(Pio->mpin()->net());
	}
#endif
}

void connect_or_merge(Pin *pin, Net *net) {
  if (!net)
    return;
  if (!pin->net())
    pin->connect(net);
  else
    pin->net()->merge(net);
}

void createConst(Design *pDesign) {
  Library *cell_lib = pDesign->find_library("cell_lib");
  Module *top = pDesign->top_module();
  Module *VCC = cell_lib->find_module("LOGIC_1");
  Module *GND = cell_lib->find_module("LOGIC_0");
  // Yosys VCC/GND
  Module *YVCC = cell_lib->find_module("VCC");
  Module *YGND = cell_lib->find_module("GND");

  Instance *iVcc = top->create_instance("VCC", VCC);
  Instance *iGnd = top->create_instance("GND", GND);

  vector<Instance *> torelease;
  for (Instance *inst : top->instances()) {
    if ((inst->down_module() == VCC && inst != iVcc) ||
        inst->down_module() == YVCC) {
      connect_or_merge(iVcc->pin(0), inst->pin(0)->net());
      torelease.push_back(inst);
    } else if ((inst->down_module() == GND && inst != iGnd) ||
               inst->down_module() == YGND) {
      connect_or_merge(iGnd->pin(0), inst->pin(0)->net());
      torelease.push_back(inst);
    }
  }
  for (Instance *inst : torelease)
    inst->release();
}
} // namespace

void MappingManager::doPtnMatch() {
  addBuf(_pDesign);
  createConst(_pDesign);
}
