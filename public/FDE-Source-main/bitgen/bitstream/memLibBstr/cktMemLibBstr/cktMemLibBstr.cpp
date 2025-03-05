#include "bitstream/memLibBstr/cktMemLibBstr/cktMemLibBstr.h"
#include "PropertyName.h"
#include "log.h"

namespace BitGen {
namespace bitstream {
using namespace FDU;

//////////////////////////////////////////////////////////////////////////
// static data members

const char *cktMemLibBstr::_1000K_MEMS[] = {
    "BRAMR4C0", "BRAMR8C0", "BRAMR12C0", "BRAMR16C0", "BRAMR20C0",
    "BRAMR4C3", "BRAMR8C3", "BRAMR12C3", "BRAMR16C3", "BRAMR20C3"};

const char *cktMemLibBstr::_3000K_PQ208_MEMS[] = {
    "BRAMR4C0",  "BRAMR8C0",  "BRAMR12C0", "BRAMR16C0",
    "BRAMR20C0", "BRAMR24C0", "BRAMR28C0", "BRAMR32C0",
    "BRAMR4C3",  "BRAMR8C3",  "BRAMR12C3", "BRAMR16C3",
    "BRAMR20C3", "BRAMR24C3", "BRAMR28C3", "BRAMR32C3"};

const char *cktMemLibBstr::_3000K_CB228_MEMS[] = {
    "LBRAMR4",  "LBRAMR8",  "LBRAMR12", "LBRAMR16", "LBRAMR20", "LBRAMR24",
    "LBRAMR28", "LBRAMR32", "RBRAMR4",  "RBRAMR8",  "RBRAMR12", "RBRAMR16",
    "RBRAMR20", "RBRAMR24", "RBRAMR28", "RBRAMR32"};

const char *cktMemLibBstr::_500KIP_FG256_MEMS[] = {
    "BMR4C1",  "BMR8C1",  "BMR12C1", "BMR16C1", "BMR20C1", "BMR24C1", "BMR28C1",
    "BMR32C1", "BMR4C2",  "BMR8C2",  "BMR12C2", "BMR16C2", "BMR20C2", "BMR24C2",
    "BMR28C2", "BMR32C2", "BMR4C3",  "BMR8C3",  "BMR12C3", "BMR16C3", "BMR20C3",
    "BMR24C3", "BMR28C3", "BMR32C3", "BMR4C4",  "BMR8C4",  "BMR12C4", "BMR16C4",
    "BMR20C4", "BMR24C4", "BMR28C4", "BMR32C4",
};
//////////////////////////////////////////////////////////////////////////
// member functions

void cktMemLibBstr::construct(const string &device, const string &package) {
  ASSERTD(_refDfotMemLib, "cktMemLib: my _refDfotMemLib is nullptr");

  dfotMemBstr *dfotMem = &_refDfotMemLib->getDfotMem();
  if (device == CHIPTYPE::FDP1000K && package == PACKAGE::PQ208) {
    for (int i = 0; i < _1000K_MEM_AMOUNT; ++i) {
      addCktMem(new cktMemBstr(dfotMem))->setName(_1000K_MEMS[i]);
    }
  } else if (device == CHIPTYPE::FDP3000K && package == PACKAGE::PQ208) {
    for (int i = 0; i < _3000K_MEM_AMOUNT; ++i) {
      addCktMem(new cktMemBstr(dfotMem))->setName(_3000K_PQ208_MEMS[i]);
    }
  } else if (device == CHIPTYPE::FDP3000K && package == PACKAGE::CB228) {
    for (int i = 0; i < _3000K_MEM_AMOUNT; ++i) {
      addCktMem(new cktMemBstr(dfotMem))->setName(_3000K_CB228_MEMS[i]);
    }
  } else if (device == CHIPTYPE::FDP500KIP && package == PACKAGE::FG256) {
    for (int i = 0; i < _500KIP_MEM_AMOUNT; ++i) {
      addCktMem(new cktMemBstr(dfotMem))->setName(_500KIP_FG256_MEMS[i]);
    }
  }
}

void cktMemLibBstr::addMemContents(const string &cktTileName,
                                   const string &contents) {
  cktMems().find(cktTileName)->addMemContents(contents);
}

} // namespace bitstream
} // namespace BitGen