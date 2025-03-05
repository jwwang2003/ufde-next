#include "bitstream/tileLibBstr/cktTileLibBstr/cktTileLibBstr.h"
#include "utils/cfgInTile.h"

#include "PropertyName.h"
#include "log.h"
#include "main/arguments/Args.h"
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>

namespace BitGen {
namespace bitstream {
using namespace boost;
using namespace boost::adaptors;
using namespace COS;
using namespace ARCH;
using namespace FDU::CHIPTYPE;

void cktTileLibBstr::construct(const std::string &device) {
  initialize(device);
  recordCktCfgs();
  recordCktPips();
  analyze();
  regulate();
  build();
  //		checkOverlaps();
}

void cktTileLibBstr::initialize(const std::string &device) {
  FPGADesign *archlib = _refCil->getArchLib();
  _fpgaScale = sizeSpan(archlib->scale().x, archlib->scale().y);
  _cktTiles.resize(_fpgaScale._rowSpan * _fpgaScale._columnSpan);
  const Library *scaleLib = archlib->find_or_create_library("arch");
  for (const Instance *inst : scaleLib->find_module(device)->instances()) {
    const ArchInstance *archInst = static_cast<const ArchInstance *>(inst);
    int bitPos =
        archInst->bit_pos().x * _fpgaScale._columnSpan + archInst->bit_pos().y;
    _cktTiles.at(bitPos) = new cktTileBstr(
        &_refDfotTileLib->getDfotTile(archInst->down_module()->name()));
    _cktTiles.at(bitPos)->setName(archInst->name());
  }
}

void cktTileLibBstr::recordCktCfgs1000K() {
  FPGADesign *archlib = _refCil->getArchLib();
  vecCfgs cfgs;
  _refCkt->listInstCfgs(cfgs);
  for (cfgElem cfg : cfgs) {
    cktTileBstr &cktTile = getTile(cfg._tileName);
    cktTile.setUsed(true);

    string siteName = cfg._siteName;
    if (archlib->is_io_bound(siteName)) {
      cfg._siteName =
          "IOB" + std::to_string((archlib->find_pad_by_name(siteName)).z);
      if (cfg._tileName == "TM" || cfg._tileName == "BM")
        cfg._siteName = "GCLK" + cfg._siteName;
    }

    std::string cfgType = cfg._cfgElemName.substr(0, 4);
    if (cfgType != "INIT" || siteName != "BRAM") {
      cktTile.addLogic(cfg);
    } else { // cfgType == "INIT" && siteName = "BRAM"
      // add to cktMemLib...
      _refCktMemLib->addMemContents(cfg._tileName, cfg._cfgElemFunc._name);
    }
  }
}

void cktTileLibBstr::recordCktCfgsFDP80K() {
  string padFORsopc;
  string tileFORsopc;
  if (args._device == FDP80K && args._sopcSwitch == true) {
    // 			padFORsopc = "N15 N16 N14 P16 T14 T13 P13 R13 N12 P12
    // R10 T10 N9 P9 R9 T9 T8 R8 N8 R7 P5 T5 R4 P4 T4 T3 P1 N1 N3 N2 N4 P8 K3
    // K2";
    // for xdl format netlist
    padFORsopc = "K15 K14 K13 L14 L13 M16 M15 M14 M13 N15 N14 N16 P16 T14 T13 "
                 "P13 R13 N12 P12 R10 T10 N9 P9 R9 T9 T8 R8 P8 N8 R7 P5 N5 R4 "
                 "P4 T4 T3 P1 N1 N3 N2 M4 K16 J14 J15 J13";
    // for xml format netlist
    tileFORsopc = "RIOIR16 BIOIC8 BIOIC7 BIOIC6 BIOIC5";
  }
  FPGADesign *archlib = _refCil->getArchLib();
  vecCfgs cfgs;
  _refCkt->listInstCfgs(cfgs);
  for (cfgElem &cfg : cfgs) {
    cktTileBstr &cktTile = getTile(cfg._tileName);
    if (cfg._cfgElemName == "PORTA_ATTR") {
      vecBits lodgerBits;
      cktTile.regulateBits(lodgerBits);
    }
    cktTile.setUsed(true);
    if (tileFORsopc.find(cktTile.getName()) != string::npos) {
      if (cfg._tileName == "TM" || cfg._tileName == "BM")
        cfg._siteName = "GCLK" + cfg._siteName;

      if (cfg._cfgElemName == "IDELMUX")
        cfg._cfgElemFunc._name = "sopc";
    }

    string siteName = cfg._siteName;

    if (archlib->is_io_bound(siteName)) {
      cfg._siteName =
          "IOB" + std::to_string((archlib->find_pad_by_name(siteName)).z);
      if (cfg._tileName == "TM" || cfg._tileName == "BM")
        cfg._siteName = "GCLK" + cfg._siteName;

      if (padFORsopc.find(siteName) != string::npos) {
        if (cfg._cfgElemName == "IDELMUX")
          cfg._cfgElemFunc._name = "sopc";
      }
    }

    if (cfg._siteName.substr(0, 7) == "BUFGMUX")
      cfg._siteName = cfg._siteName.substr(0, 8); //?
    if (cfg._siteName.substr(0, 6) == "BRAM16")
      cfg._siteName = cfg._siteName.substr(0, 6); //?

    std::string cfgType = cfg._cfgElemName.substr(0, 5);

    if (cfg._cfgElemName == "INIT_A" || cfg._cfgElemName == "INIT_B")
      cktTile.addLogic(cfg);
    else if (cfg._siteName == "BRAM16") {
      cktTileBstr *pcktTile = &getTile(cfg._tileName);
      BitGen::bitstream::cktTileBstr::tileBstr *ckttileBstr =
          &pcktTile->getTileBstr();
      FDU::cil_lib::bramLib *bramlib = _refCil->getBramLib(); //?
      int bram_init[256];
      int bram_p[32];
      for (FDU::cil_lib::Bram *bram : bramlib->brams()) {
        int bramAddr = bram->getBramAddr();
        int bl = bram->getBl();
        std::string type = bram->getType();

        if (type == "INIT")
          bram_init[bramAddr] = bl;
        else
          bram_p[bramAddr] = bl;
      }

      if (cfgType == "INITP") {
        BitGen::bitstream::cktTileBstr::tileBstr &ckttileBstr_i =
            pcktTile->getTileBstr();
        if (ckttileBstr_i.empty()) {
          for (int idx_sram = 0; idx_sram < 320 * 8 * 8; ++idx_sram) {
            ckttileBstr->push_back(0);
          }
        }
        std::string init = cfg._cfgElemName.substr(6, 2);
        int frm;
        sscanf(init.c_str(), "%x", &frm);
        int word_bl[256]; // ��ŵ�·�ļ�xdl�������Ϣ
        std::string word = cfg._cfgElemFunc._name;
        int id = 255;
        for (std::string::size_type ix = 0; ix != word.size(); ++ix) {
          std::string content = word.substr(ix, 1);
          int hex;
          sscanf(content.c_str(), "%x", &hex);
          for (int flag = 0x8; flag != 0; flag >>= 1, --id) {
            word_bl[id] = hex & flag ? 1 : 0;
          }
        }
        for (int ix_tileBstr = 0; ix_tileBstr < 8; ++ix_tileBstr) {
          for (int idx = 0; idx < 32; ++idx) {
            ckttileBstr_i[frm * 8 + ix_tileBstr + 64 * bram_p[idx]] =
                word_bl[idx + ix_tileBstr * 32];
          }
        }
      }

      else if (cfgType == "INIT_") {
        BitGen::bitstream::cktTileBstr::tileBstr &ckttileBstr_i =
            pcktTile->getTileBstr();
        if (ckttileBstr_i.empty()) { // Ԥ����У��λ����Ҫ��ʼ�������
          for (int idx_sram = 0; idx_sram < 320 * 8 * 8; ++idx_sram) {
            ckttileBstr->push_back(0);
          }
        }
        std::string init = cfg._cfgElemName.substr(5, 2);
        int frm;
        sscanf(init.c_str(), "%x", &frm);
        int word_bl[256];
        std::string word = cfg._cfgElemFunc._name;
        int id = 255;
        for (std::string::size_type ix = 0; ix != word.size(); ++ix) {
          std::string content = word.substr(ix, 1);
          int hex;
          sscanf(content.c_str(), "%x", &hex);
          for (int flag = 0x8; flag != 0; flag >>= 1, --id) {
            word_bl[id] = hex & flag ? 1 : 0;
          }
        }

        for (int idx = 0; idx < 256; ++idx) {
          std::vector<int>::size_type position = frm + 64 * bram_init[idx];
          ckttileBstr_i[position] = word_bl[idx];
        }
      } else {
        pcktTile->addLogic(cfg);
      }
    } else {
      cktTile.addLogic(cfg);
    }
  }
}

void cktTileLibBstr::recordCktCfgs() {
  if (args._device == CHIPTYPE::FDP1000K || args._device == CHIPTYPE::FDP3000K)
    recordCktCfgs1000K();
  else if (args._device == CHIPTYPE::FDP80K ||
           args._device == CHIPTYPE::FDP500K ||
           args._device == CHIPTYPE::FDP500KIP)
    recordCktCfgsFDP80K();
}

void cktTileLibBstr::recordCktPips() {
  vecPips pips;
  _refCkt->listNetPips(pips);
  for (pipInfo &pip : pips) {
    cktTileBstr *cktTile = &getTile(pip._tileName);
    cktTile->setUsed(true);

    routeInfo &route = cktTile->getRoutes().addRoute(routeInfo());
    route._siteName = cktTile->getRefTile()->getGRMName();
    route._srcNet = pip._srcNet;
    route._snkNet = pip._snkNet;
  }
}

void cktTileLibBstr::analyze() {
  for (cktTileBstr *cktTile : cktTiles())
    cktTile->analyzeBits();
}

void cktTileLibBstr::regulate() {
  int rows = _fpgaScale._rowSpan, cols = _fpgaScale._columnSpan, tileIdx = 0;
  vecBits lodgerBits;
  for (cktTileBstr *cktTile : cktTiles()) {
    int row = tileIdx / cols;
    int col = tileIdx - row * cols;
    cktTile->regulateBits(lodgerBits);
    for (bitTile &bit : lodgerBits) {
      sizeSpan offset = bit._tileOffset;
      int targetRow = row + offset._rowSpan;
      int targetCol = col + offset._columnSpan;
      ASSERT(targetRow >= 0 && targetRow < rows,
             "ckt TileLib: Row of tile out of range after handle offset");
      ASSERT(targetCol >= 0 && targetCol < cols,
             "ckt TileLib: Col of tile out of range after handle offset");
      int targetIdx = targetRow * cols + targetCol;
      bit._tileOffset = sizeSpan(0, 0);
      _cktTiles.at(targetIdx)->addBit(bit);
      _cktTiles.at(targetIdx)->setUsed(true);
    }
    ++tileIdx;
    lodgerBits.clear();
  }
  ASSERTD(tileIdx == rows * cols,
          "cktTileLib: total number of tiles in FDP1000K errors");
}

void cktTileLibBstr::build() {
  for (cktTileBstr *cktTile : cktTiles())
    cktTile->buildBitArry();
}

sizeSpan cktTileLibBstr::getColBits(std::vector<int> &colBits, int tileCol) {
  int rows = _fpgaScale._rowSpan, cols = _fpgaScale._columnSpan;
  ASSERTD(tileCol >= 0 && tileCol < cols,
          "cktTileLib: col of tile out of range");
  int bitRowAmnt = 0, bitColAmnt = -1;
  for (int row = 0; row < rows; ++row) {
    sizeSpan bitArrySize = getTile(sizeSpan(row, tileCol)).getBstrSize();
    if (bitColAmnt == -1) {
      bitColAmnt = bitArrySize._columnSpan;
    }
    ASSERTD(bitColAmnt == bitArrySize._columnSpan,
            "cktTileLib: bit-array not align at tile ... " +
                boost::lexical_cast<string>(sizeSpan(row, tileCol)));
    bitRowAmnt += bitArrySize._rowSpan;
  }
  for (int bitCol = 0; bitCol < bitColAmnt; ++bitCol) {
    getFRMBits(colBits, tileCol, bitCol);
  }
  return sizeSpan(bitRowAmnt, bitColAmnt);
}

void cktTileLibBstr::checkOverlaps() {
  for (cktTileBstr *cktTile : cktTiles())
    if (cktTile->checkOverlaps())
      FDU_LOG(WARN) << Warning("bitgen: overlap bits exist at circuit tile - " +
                               cktTile->getName());
}

} // namespace bitstream
} // namespace BitGen
