#include "bitstream/bstrGenerate/bstrGener.h"
#include "bitstream/logWriter/xmlWriter/xmlWriter.h"
#include "main/arguments/Args.h"

#include <boost/range/adaptors.hpp>
#include <iostream>

namespace BitGen {
namespace bitstream {
using namespace std;
using namespace boost;
using namespace boost::adaptors;
using namespace BitGen::bitstream::xmlio;

//////////////////////////////////////////////////////////////////////////
// static data members

const char *bstrGener::_BIT_FILE = ".bit";
const char *bstrGener::_FRM_FILE = ".frm";
const char *bstrGener::_LOG_FILE = ".xml";
const char *bstrGener::_OVL_FILE = ".ovl";
const char *bstrGener::_ARY_FILE = ".ary";
const char *bstrGener::_DFOT_LOG_PATH = "\\default\\";
const char *bstrGener::_CKT_LOG_PATH = "\\circuit\\";

//////////////////////////////////////////////////////////////////////////
// function members

void bstrGener::generate(const std::string &bstrPath, const std::string &device,
                         const std::string &package) {
  //////////////////////////////////////////////////////////////////////////
  // default references
  FDU_LOG(INFO) << Info("> Construct FPGA chip default configurations ... ");
  _dfotMems.construct();
  _dfotTiles.construct();

  //////////////////////////////////////////////////////////////////////////
  // used in circuit
  FDU_LOG(INFO) << Info("> Construct user-design configurations ... ");
  _cktMems.construct(device, package);
  _cktTiles.construct(device);

  //////////////////////////////////////////////////////////////////////////
  // generate bitstream
  FDU_LOG(INFO) << Info("> Generate bitstream files ... ");
  _CMDLoader.bstrGen(bstrPath /* + _BIT_FILE, device*/);
  if (args._frmSwitch)
    _FRMLoader.bstrGen(bstrPath.substr(0, bstrPath.rfind('.')) + _FRM_FILE);
}

void bstrGener::showLog(const std::string &workDir) {
  string dfotPath = workDir + _DFOT_LOG_PATH;
  string cktPath = workDir + _CKT_LOG_PATH;
  // string cktPath  = workDir + "\\";
  for (const dfotTileBstr *dfotTile : _dfotTiles.dfotTiles()) {
    xmlWriter nmlWriter(xmlWriter::NORMAL, *dfotTile),
        ovlWriter(xmlWriter::OVERLAPS, *dfotTile);
    string fName = dfotPath + dfotTile->getName();
    nmlWriter.write(fName + _LOG_FILE);

    if (dfotTile->hasOverlaps())
      ovlWriter.write(fName + _OVL_FILE);
  }
  for (const cktTileBstr *cktTile : _cktTiles.cktTiles())
    if (cktTile->isUsed()) {
      xmlWriter nmlWriter(xmlWriter::NORMAL, *cktTile),
          ovlWriter(xmlWriter::OVERLAPS, *cktTile);
      string fName = cktPath + cktTile->getName();
      nmlWriter.write(fName + _LOG_FILE);
      cktTile->exportArry(fName + _ARY_FILE);

      if (cktTile->hasOverlaps())
        ovlWriter.write(fName + _OVL_FILE);
    }
}

} // namespace bitstream
} // namespace BitGen