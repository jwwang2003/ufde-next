#ifndef _BSTRGENER_H_
#define _BSTRGENER_H_

#include "bitstream/bstrGenerate/CMDLoader/CMDLoader.h"
#include "bitstream/bstrGenerate/CMDLoader/CMDLoaderBase.h"
#include "bitstream/bstrGenerate/FRMLoader/FRMLoaderBase.h"
#include "bitstream/memLibBstr/cktMemLibBstr/cktMemLibBstr.h"
#include "bitstream/tileLibBstr/cktTileLibBstr/cktTileLibBstr.h"
#include "cil/cilLib.h"
#include "circuit/circuit.h"

namespace BitGen {
namespace bitstream {
using namespace FDU::cil_lib;
using namespace BitGen::circuit;

class bstrGener {
public:
  static const char *_BIT_FILE;
  static const char *_FRM_FILE;
  static const char *_LOG_FILE;
  static const char *_OVL_FILE;
  static const char *_ARY_FILE;
  static const char *_DFOT_LOG_PATH;
  static const char *_CKT_LOG_PATH;

private:
  dfotTileLibBstr _dfotTiles;
  dfotMemLibBstr _dfotMems;
  cktTileLibBstr _cktTiles;
  cktMemLibBstr _cktMems;
  CMDLoader _CMDLoader;
  FRMLoaderBase _FRMLoader;

public:
  bstrGener(Circuit *refCkt, cilLibrary *refCilLib)
      : _dfotTiles(refCilLib->getTileLib()),
        _cktTiles(refCilLib, refCkt, &_dfotTiles, &_cktMems),
        _cktMems(&_dfotMems),
        _CMDLoader(&_cktMems, &_cktTiles, refCilLib->getMajorLib(),
                   refCilLib->getbstrCMDLib()),
        _FRMLoader(&_cktMems, &_cktTiles, refCilLib->getMajorLib(),
                   refCilLib->getbstrCMDLib()) {}

  void generate(const std::string &bstrPath, const std::string &device,
                const std::string &package);
  void showLog(const std::string &workDir);
};

} // namespace bitstream
} // namespace BitGen

#endif