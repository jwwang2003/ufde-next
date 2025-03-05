#include "bitstream/bstrGenerate/CMDLoader/CMDLoader.h"

namespace BitGen {
namespace bitstream {

void CMDLoader::bstrGen(const std::string &outputFile) {
  for (FDU::cil_lib::bstrCMD *bstrcmd : _refbstrCMDLib->bstrCMDs()) {
    std::string cmd = bstrcmd->getCmd();
    std::string parameter = bstrcmd->getParameter();
    int pos;
    if (cmd == "bsHeader")
      bsHeader();
    else if (cmd == "adjustSYNC")
      adjustSYNC(parameter);
    else if (cmd == "insertCMD") {
      pos = parameter.find(",", 0);
      std::string parameter1 = parameter.substr(0, pos);
      std::string parameter2 = "";
      if (pos != -1)
        parameter2 = parameter.substr(pos + 1, parameter.size() - 1);
      parameter1.erase(0, parameter1.find_first_not_of(" "));
      insertCMD(parameter1, parameter2);
    } else if (cmd == "setFRMLen")
      setFRMLen(_refbstrCMDLib->getbstrParameter("FRMLen").getParameterValue());
    else if (cmd == "setOption")
      setOption(parameter);
    else if (cmd == "shapeMask")
      shapeMask(parameter);
    else if (cmd == "dummy")
      dummy();
    else if (cmd == "writeNomalTiles") {
      int major_shift =
          _refbstrCMDLib->getbstrParameter("major_shift").getParameterValue();
      int initialNum =
          _refbstrCMDLib->getbstrParameter("initialNum").getParameterValue();
      int bits_per_grp_reversed =
          _refbstrCMDLib->getbstrParameter("bits_per_grp_reversed")
              .getParameterValue();
      writeNomalTiles(major_shift, bits_per_grp_reversed, initialNum);
    } else if (cmd == "writeMem") {
      int wrdsAmnt_shift = _refbstrCMDLib->getbstrParameter("wrdsAmnt_shift")
                               .getParameterValue();
      int fillblank =
          _refbstrCMDLib->getbstrParameter("fillblank").getParameterValue();
      int mem_amount =
          _refbstrCMDLib->getbstrParameter("mem_amount").getParameterValue();
      writeMem(mem_amount, wrdsAmnt_shift, fillblank);
    } else if (cmd == "appointCTL")
      appointCTL(parameter);
  }
  exportBitFile(outputFile);
}

} // namespace bitstream
} // namespace BitGen