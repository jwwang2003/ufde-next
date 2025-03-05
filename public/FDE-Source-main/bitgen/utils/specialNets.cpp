#include "utils/specialNets.h"
#include "PropertyName.h"
#include "exception/exceptions.h"
#include "main/arguments/Args.h"
#include "utils/sizeSpan.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace FDU;

sizeSpan INNER_SCALE;
map<string, int> SPECIAL_NETS;
map<string, int> MAYBE_BIDIRECTIONAL_NETS;

void INIT_INNER_SCALE() {
  if (args._device == CHIPTYPE::FDP1000K) {
    INNER_SCALE = sizeSpan(20, 30);
  } else if (args._device == CHIPTYPE::FDP3000K) {
    INNER_SCALE = sizeSpan(32, 48);
  } else
    throw CktException("Special Nets: invalid chip name");
}

void INIT_SPECIAL_NETS() {
  SPECIAL_NETS["EA0"] = -3;
  SPECIAL_NETS["EA1"] = -3;
  SPECIAL_NETS["EA2"] = -3;
  SPECIAL_NETS["EA3"] = -3;
  SPECIAL_NETS["EA4"] = -3;
  SPECIAL_NETS["EA5"] = -3;
  SPECIAL_NETS["EA6"] = -3;
  SPECIAL_NETS["EA7"] = -3;
  SPECIAL_NETS["EA8"] = -3;
  SPECIAL_NETS["EA9"] = -3;
  SPECIAL_NETS["EA10"] = -3;
  SPECIAL_NETS["EA11"] = -3;
  SPECIAL_NETS["EA12"] = -3;
  SPECIAL_NETS["EA13"] = -3;
  SPECIAL_NETS["EA14"] = -3;
  SPECIAL_NETS["EA15"] = -3;
  SPECIAL_NETS["EA16"] = -3;
  SPECIAL_NETS["EA17"] = -3;
  SPECIAL_NETS["EA18"] = -3;
  SPECIAL_NETS["EA19"] = -3;
  SPECIAL_NETS["EA20"] = -3;
  SPECIAL_NETS["EA21"] = -3;
  SPECIAL_NETS["EA22"] = -3;
  SPECIAL_NETS["EA23"] = -3;
  SPECIAL_NETS["H6EA0"] = -3;
  SPECIAL_NETS["H6EA1"] = -3;
  SPECIAL_NETS["H6EA2"] = -3;
  SPECIAL_NETS["H6EA3"] = -3;
  SPECIAL_NETS["H6BA0"] = -3;
  SPECIAL_NETS["H6BA1"] = -3;
  SPECIAL_NETS["H6BA2"] = -3;
  SPECIAL_NETS["H6BA3"] = -3;
  SPECIAL_NETS["H6MA0"] = -3;
  SPECIAL_NETS["H6MA1"] = -3;
  SPECIAL_NETS["H6MA2"] = -3;
  SPECIAL_NETS["H6MA3"] = -3;
  SPECIAL_NETS["H6DA0"] = -3;
  SPECIAL_NETS["H6DA1"] = -3;
  SPECIAL_NETS["H6DA2"] = -3;
  SPECIAL_NETS["H6DA3"] = -3;
  SPECIAL_NETS["LHA0"] = -3;
  SPECIAL_NETS["LHA3"] = -3;
  SPECIAL_NETS["LHA6"] = -3;
  SPECIAL_NETS["LHA9"] = -3;
  SPECIAL_NETS["GCLK_IOBA0"] = -3;
  SPECIAL_NETS["GCLK_IOBA1"] = -3;
  SPECIAL_NETS["GCLK_IOBA2"] = -3;
  SPECIAL_NETS["GCLK_IOBA3"] = -3;
  SPECIAL_NETS["GCLK_CLBA0"] = -3;
  SPECIAL_NETS["GCLK_CLBA1"] = -3;
  SPECIAL_NETS["GCLK_CLBA2"] = -3;
  SPECIAL_NETS["GCLK_CLBA3"] = -3;
  SPECIAL_NETS["RADDRN0"] = -3;
  SPECIAL_NETS["RADDRN1"] = -3;
  SPECIAL_NETS["RADDRN2"] = -3;
  SPECIAL_NETS["RADDRN3"] = -3;
  SPECIAL_NETS["RADDRN4"] = -3;
  SPECIAL_NETS["RADDRN5"] = -3;
  SPECIAL_NETS["RADDRN6"] = -3;
  SPECIAL_NETS["RADDRN7"] = -3;
  SPECIAL_NETS["RADDRN8"] = -3;
  SPECIAL_NETS["RADDRN9"] = -3;
  SPECIAL_NETS["RADDRN10"] = -3;
  SPECIAL_NETS["RADDRN11"] = -3;
  SPECIAL_NETS["RADDRN12"] = -3;
  SPECIAL_NETS["RADDRN13"] = -3;
  SPECIAL_NETS["RADDRN14"] = -3;
  SPECIAL_NETS["RADDRN15"] = -3;
  SPECIAL_NETS["RADDRN16"] = -3;
  SPECIAL_NETS["RADDRN17"] = -3;
  SPECIAL_NETS["RADDRN18"] = -3;
  SPECIAL_NETS["RADDRN19"] = -3;
  SPECIAL_NETS["RADDRN20"] = -3;
  SPECIAL_NETS["RADDRN21"] = -3;
  SPECIAL_NETS["RADDRN22"] = -3;
  SPECIAL_NETS["RADDRN23"] = -3;
  SPECIAL_NETS["RADDRN24"] = -3;
  SPECIAL_NETS["RADDRN25"] = -3;
  SPECIAL_NETS["RADDRN26"] = -3;
  SPECIAL_NETS["RADDRN27"] = -3;
  SPECIAL_NETS["RADDRN28"] = -3;
  SPECIAL_NETS["RADDRN29"] = -3;
  SPECIAL_NETS["RADDRN30"] = -3;
  SPECIAL_NETS["RADDRN31"] = -3;
  SPECIAL_NETS["RDINN0"] = -3;
  SPECIAL_NETS["RDINN1"] = -3;
  SPECIAL_NETS["RDINN2"] = -3;
  SPECIAL_NETS["RDINN3"] = -3;
  SPECIAL_NETS["RDINN4"] = -3;
  SPECIAL_NETS["RDINN5"] = -3;
  SPECIAL_NETS["RDINN6"] = -3;
  SPECIAL_NETS["RDINN7"] = -3;
  SPECIAL_NETS["RDINN8"] = -3;
  SPECIAL_NETS["RDINN9"] = -3;
  SPECIAL_NETS["RDINN10"] = -3;
  SPECIAL_NETS["RDINN11"] = -3;
  SPECIAL_NETS["RDINN12"] = -3;
  SPECIAL_NETS["RDINN13"] = -3;
  SPECIAL_NETS["RDINN14"] = -3;
  SPECIAL_NETS["RDINN15"] = -3;
  SPECIAL_NETS["RDINN16"] = -3;
  SPECIAL_NETS["RDINN17"] = -3;
  SPECIAL_NETS["RDINN18"] = -3;
  SPECIAL_NETS["RDINN19"] = -3;
  SPECIAL_NETS["RDINN20"] = -3;
  SPECIAL_NETS["RDINN21"] = -3;
  SPECIAL_NETS["RDINN22"] = -3;
  SPECIAL_NETS["RDINN23"] = -3;
  SPECIAL_NETS["RDINN24"] = -3;
  SPECIAL_NETS["RDINN25"] = -3;
  SPECIAL_NETS["RDINN26"] = -3;
  SPECIAL_NETS["RDINN27"] = -3;
  SPECIAL_NETS["RDINN28"] = -3;
  SPECIAL_NETS["RDINN29"] = -3;
  SPECIAL_NETS["RDINN30"] = -3;
  SPECIAL_NETS["RDINN31"] = -3;
  SPECIAL_NETS["RDOUTN0"] = -3;
  SPECIAL_NETS["RDOUTN1"] = -3;
  SPECIAL_NETS["RDOUTN2"] = -3;
  SPECIAL_NETS["RDOUTN3"] = -3;
  SPECIAL_NETS["RDOUTN4"] = -3;
  SPECIAL_NETS["RDOUTN5"] = -3;
  SPECIAL_NETS["RDOUTN6"] = -3;
  SPECIAL_NETS["RDOUTN7"] = -3;
  SPECIAL_NETS["RDOUTN8"] = -3;
  SPECIAL_NETS["RDOUTN9"] = -3;
  SPECIAL_NETS["RDOUTN10"] = -3;
  SPECIAL_NETS["RDOUTN11"] = -3;
  SPECIAL_NETS["RDOUTN12"] = -3;
  SPECIAL_NETS["RDOUTN13"] = -3;
  SPECIAL_NETS["RDOUTN14"] = -3;
  SPECIAL_NETS["RDOUTN15"] = -3;
  SPECIAL_NETS["RDOUTN16"] = -3;
  SPECIAL_NETS["RDOUTN17"] = -3;
  SPECIAL_NETS["RDOUTN18"] = -3;
  SPECIAL_NETS["RDOUTN19"] = -3;
  SPECIAL_NETS["RDOUTN20"] = -3;
  SPECIAL_NETS["RDOUTN21"] = -3;
  SPECIAL_NETS["RDOUTN22"] = -3;
  SPECIAL_NETS["RDOUTN23"] = -3;
  SPECIAL_NETS["RDOUTN24"] = -3;
  SPECIAL_NETS["RDOUTN25"] = -3;
  SPECIAL_NETS["RDOUTN26"] = -3;
  SPECIAL_NETS["RDOUTN27"] = -3;
  SPECIAL_NETS["RDOUTN28"] = -3;
  SPECIAL_NETS["RDOUTN29"] = -3;
  SPECIAL_NETS["RDOUTN30"] = -3;
  SPECIAL_NETS["RDOUTN31"] = -3;
  SPECIAL_NETS["DIA0"] = -3;
  SPECIAL_NETS["DIA2"] = -3;
  SPECIAL_NETS["DIA8"] = -3;
  SPECIAL_NETS["DIA10"] = -3;
  SPECIAL_NETS["DIB0"] = -3;
  SPECIAL_NETS["DIB2"] = -3;
  SPECIAL_NETS["DIB8"] = -3;
  SPECIAL_NETS["DIB10"] = -3;
  SPECIAL_NETS["DOA0"] = -3;
  SPECIAL_NETS["DOA4"] = -3;
  SPECIAL_NETS["DOA8"] = -3;
  SPECIAL_NETS["DOA12"] = -3;
  SPECIAL_NETS["DOB0"] = -3;
  SPECIAL_NETS["DOB4"] = -3;
  SPECIAL_NETS["DOB8"] = -3;
  SPECIAL_NETS["DOB12"] = -3;

  SPECIAL_NETS["EB0"] = -2;
  SPECIAL_NETS["EB1"] = -2;
  SPECIAL_NETS["EB2"] = -2;
  SPECIAL_NETS["EB3"] = -2;
  SPECIAL_NETS["EB4"] = -2;
  SPECIAL_NETS["EB5"] = -2;
  SPECIAL_NETS["EB6"] = -2;
  SPECIAL_NETS["EB7"] = -2;
  SPECIAL_NETS["EB8"] = -2;
  SPECIAL_NETS["EB9"] = -2;
  SPECIAL_NETS["EB10"] = -2;
  SPECIAL_NETS["EB11"] = -2;
  SPECIAL_NETS["EB12"] = -2;
  SPECIAL_NETS["EB13"] = -2;
  SPECIAL_NETS["EB14"] = -2;
  SPECIAL_NETS["EB15"] = -2;
  SPECIAL_NETS["EB16"] = -2;
  SPECIAL_NETS["EB17"] = -2;
  SPECIAL_NETS["EB18"] = -2;
  SPECIAL_NETS["EB19"] = -2;
  SPECIAL_NETS["EB20"] = -2;
  SPECIAL_NETS["EB21"] = -2;
  SPECIAL_NETS["EB22"] = -2;
  SPECIAL_NETS["EB23"] = -2;
  SPECIAL_NETS["H6EB0"] = -2;
  SPECIAL_NETS["H6EB1"] = -2;
  SPECIAL_NETS["H6EB2"] = -2;
  SPECIAL_NETS["H6EB3"] = -2;
  SPECIAL_NETS["H6BB0"] = -2;
  SPECIAL_NETS["H6BB1"] = -2;
  SPECIAL_NETS["H6BB2"] = -2;
  SPECIAL_NETS["H6BB3"] = -2;
  SPECIAL_NETS["H6MB0"] = -2;
  SPECIAL_NETS["H6MB1"] = -2;
  SPECIAL_NETS["H6MB2"] = -2;
  SPECIAL_NETS["H6MB3"] = -2;
  SPECIAL_NETS["H6DB0"] = -2;
  SPECIAL_NETS["H6DB1"] = -2;
  SPECIAL_NETS["H6DB2"] = -2;
  SPECIAL_NETS["H6DB3"] = -2;
  SPECIAL_NETS["LHB0"] = -2;
  SPECIAL_NETS["LHB3"] = -2;
  SPECIAL_NETS["LHB6"] = -2;
  SPECIAL_NETS["LHB9"] = -2;
  SPECIAL_NETS["GCLK_IOBB0"] = -2;
  SPECIAL_NETS["GCLK_IOBB1"] = -2;
  SPECIAL_NETS["GCLK_IOBB2"] = -2;
  SPECIAL_NETS["GCLK_IOBB3"] = -2;
  SPECIAL_NETS["GCLK_CLBB0"] = -2;
  SPECIAL_NETS["GCLK_CLBB1"] = -2;
  SPECIAL_NETS["GCLK_CLBB2"] = -2;
  SPECIAL_NETS["GCLK_CLBB3"] = -2;
  SPECIAL_NETS["DIA1"] = -2;
  SPECIAL_NETS["DIA3"] = -2;
  SPECIAL_NETS["DIA9"] = -2;
  SPECIAL_NETS["DIA11"] = -2;
  SPECIAL_NETS["DIB1"] = -2;
  SPECIAL_NETS["DIB3"] = -2;
  SPECIAL_NETS["DIB9"] = -2;
  SPECIAL_NETS["DIB11"] = -2;
  SPECIAL_NETS["DOA1"] = -2;
  SPECIAL_NETS["DOA5"] = -2;
  SPECIAL_NETS["DOA9"] = -2;
  SPECIAL_NETS["DOA13"] = -2;
  SPECIAL_NETS["DOB1"] = -2;
  SPECIAL_NETS["DOB5"] = -2;
  SPECIAL_NETS["DOB9"] = -2;
  SPECIAL_NETS["DOB13"] = -2;
  SPECIAL_NETS["ADDRA0"] = -2;
  SPECIAL_NETS["ADDRA1"] = -2;
  SPECIAL_NETS["ADDRA2"] = -2;
  SPECIAL_NETS["ADDRA3"] = -2;
  SPECIAL_NETS["ADDRB0"] = -2;
  SPECIAL_NETS["ADDRB1"] = -2;
  SPECIAL_NETS["ADDRB2"] = -2;
  SPECIAL_NETS["ADDRB3"] = -2;
  SPECIAL_NETS["CLKA"] = -2;
  SPECIAL_NETS["WEA"] = -2;
  SPECIAL_NETS["SELA"] = -2;
  SPECIAL_NETS["RSTA"] = -2;

  SPECIAL_NETS["EC0"] = -1;
  SPECIAL_NETS["EC1"] = -1;
  SPECIAL_NETS["EC2"] = -1;
  SPECIAL_NETS["EC3"] = -1;
  SPECIAL_NETS["EC4"] = -1;
  SPECIAL_NETS["EC5"] = -1;
  SPECIAL_NETS["EC6"] = -1;
  SPECIAL_NETS["EC7"] = -1;
  SPECIAL_NETS["EC8"] = -1;
  SPECIAL_NETS["EC9"] = -1;
  SPECIAL_NETS["EC10"] = -1;
  SPECIAL_NETS["EC11"] = -1;
  SPECIAL_NETS["EC12"] = -1;
  SPECIAL_NETS["EC13"] = -1;
  SPECIAL_NETS["EC14"] = -1;
  SPECIAL_NETS["EC15"] = -1;
  SPECIAL_NETS["EC16"] = -1;
  SPECIAL_NETS["EC17"] = -1;
  SPECIAL_NETS["EC18"] = -1;
  SPECIAL_NETS["EC19"] = -1;
  SPECIAL_NETS["EC20"] = -1;
  SPECIAL_NETS["EC21"] = -1;
  SPECIAL_NETS["EC22"] = -1;
  SPECIAL_NETS["EC23"] = -1;
  SPECIAL_NETS["H6EC0"] = -1;
  SPECIAL_NETS["H6EC1"] = -1;
  SPECIAL_NETS["H6EC2"] = -1;
  SPECIAL_NETS["H6EC3"] = -1;
  SPECIAL_NETS["H6BC0"] = -1;
  SPECIAL_NETS["H6BC1"] = -1;
  SPECIAL_NETS["H6BC2"] = -1;
  SPECIAL_NETS["H6BC3"] = -1;
  SPECIAL_NETS["H6MC0"] = -1;
  SPECIAL_NETS["H6MC1"] = -1;
  SPECIAL_NETS["H6MC2"] = -1;
  SPECIAL_NETS["H6MC3"] = -1;
  SPECIAL_NETS["H6DC0"] = -1;
  SPECIAL_NETS["H6DC1"] = -1;
  SPECIAL_NETS["H6DC2"] = -1;
  SPECIAL_NETS["H6DC3"] = -1;
  SPECIAL_NETS["LHC0"] = -1;
  SPECIAL_NETS["LHC3"] = -1;
  SPECIAL_NETS["LHC6"] = -1;
  SPECIAL_NETS["LHC9"] = -1;
  SPECIAL_NETS["GCLK_IOBC0"] = -1;
  SPECIAL_NETS["GCLK_IOBC1"] = -1;
  SPECIAL_NETS["GCLK_IOBC2"] = -1;
  SPECIAL_NETS["GCLK_IOBC3"] = -1;
  SPECIAL_NETS["GCLK_CLBC0"] = -1;
  SPECIAL_NETS["GCLK_CLBC1"] = -1;
  SPECIAL_NETS["GCLK_CLBC2"] = -1;
  SPECIAL_NETS["GCLK_CLBC3"] = -1;
  SPECIAL_NETS["DIA4"] = -1;
  SPECIAL_NETS["DIA5"] = -1;
  SPECIAL_NETS["DIA12"] = -1;
  SPECIAL_NETS["DIA13"] = -1;
  SPECIAL_NETS["DIB4"] = -1;
  SPECIAL_NETS["DIB5"] = -1;
  SPECIAL_NETS["DIB12"] = -1;
  SPECIAL_NETS["DIB13"] = -1;
  SPECIAL_NETS["DOA2"] = -1;
  SPECIAL_NETS["DOA6"] = -1;
  SPECIAL_NETS["DOA10"] = -1;
  SPECIAL_NETS["DOA14"] = -1;
  SPECIAL_NETS["DOB2"] = -1;
  SPECIAL_NETS["DOB6"] = -1;
  SPECIAL_NETS["DOB10"] = -1;
  SPECIAL_NETS["DOB14"] = -1;
  SPECIAL_NETS["ADDRA4"] = -1;
  SPECIAL_NETS["ADDRA5"] = -1;
  SPECIAL_NETS["ADDRA6"] = -1;
  SPECIAL_NETS["ADDRA7"] = -1;
  SPECIAL_NETS["ADDRB4"] = -1;
  SPECIAL_NETS["ADDRB5"] = -1;
  SPECIAL_NETS["ADDRB6"] = -1;
  SPECIAL_NETS["ADDRB7"] = -1;
  SPECIAL_NETS["CLKB"] = -1;
  SPECIAL_NETS["WEB"] = -1;
  SPECIAL_NETS["SELB"] = -1;
  SPECIAL_NETS["RSTB"] = -1;

  SPECIAL_NETS["ED0"] = 0;
  SPECIAL_NETS["ED1"] = 0;
  SPECIAL_NETS["ED2"] = 0;
  SPECIAL_NETS["ED3"] = 0;
  SPECIAL_NETS["ED4"] = 0;
  SPECIAL_NETS["ED5"] = 0;
  SPECIAL_NETS["ED6"] = 0;
  SPECIAL_NETS["ED7"] = 0;
  SPECIAL_NETS["ED8"] = 0;
  SPECIAL_NETS["ED9"] = 0;
  SPECIAL_NETS["ED10"] = 0;
  SPECIAL_NETS["ED11"] = 0;
  SPECIAL_NETS["ED12"] = 0;
  SPECIAL_NETS["ED13"] = 0;
  SPECIAL_NETS["ED14"] = 0;
  SPECIAL_NETS["ED15"] = 0;
  SPECIAL_NETS["ED16"] = 0;
  SPECIAL_NETS["ED17"] = 0;
  SPECIAL_NETS["ED18"] = 0;
  SPECIAL_NETS["ED19"] = 0;
  SPECIAL_NETS["ED20"] = 0;
  SPECIAL_NETS["ED21"] = 0;
  SPECIAL_NETS["ED22"] = 0;
  SPECIAL_NETS["ED23"] = 0;
  SPECIAL_NETS["H6ED0"] = 0;
  SPECIAL_NETS["H6ED1"] = 0;
  SPECIAL_NETS["H6ED2"] = 0;
  SPECIAL_NETS["H6ED3"] = 0;
  SPECIAL_NETS["H6BD0"] = 0;
  SPECIAL_NETS["H6BD1"] = 0;
  SPECIAL_NETS["H6BD2"] = 0;
  SPECIAL_NETS["H6BD3"] = 0;
  SPECIAL_NETS["H6MD0"] = 0;
  SPECIAL_NETS["H6MD1"] = 0;
  SPECIAL_NETS["H6MD2"] = 0;
  SPECIAL_NETS["H6MD3"] = 0;
  SPECIAL_NETS["H6DD0"] = 0;
  SPECIAL_NETS["H6DD1"] = 0;
  SPECIAL_NETS["H6DD2"] = 0;
  SPECIAL_NETS["H6DD3"] = 0;
  SPECIAL_NETS["LHD0"] = 0;
  SPECIAL_NETS["LHD3"] = 0;
  SPECIAL_NETS["LHD6"] = 0;
  SPECIAL_NETS["LHD9"] = 0;
  SPECIAL_NETS["GCLK_IOBD0"] = 0;
  SPECIAL_NETS["GCLK_IOBD1"] = 0;
  SPECIAL_NETS["GCLK_IOBD2"] = 0;
  SPECIAL_NETS["GCLK_IOBD3"] = 0;
  SPECIAL_NETS["GCLK_CLBD0"] = 0;
  SPECIAL_NETS["GCLK_CLBD1"] = 0;
  SPECIAL_NETS["GCLK_CLBD2"] = 0;
  SPECIAL_NETS["GCLK_CLBD3"] = 0;
  SPECIAL_NETS["DIA6"] = 0;
  SPECIAL_NETS["DIA7"] = 0;
  SPECIAL_NETS["DIA14"] = 0;
  SPECIAL_NETS["DIA15"] = 0;
  SPECIAL_NETS["DIB6"] = 0;
  SPECIAL_NETS["DIB7"] = 0;
  SPECIAL_NETS["DIB14"] = 0;
  SPECIAL_NETS["DIB15"] = 0;
  SPECIAL_NETS["DOA3"] = 0;
  SPECIAL_NETS["DOA7"] = 0;
  SPECIAL_NETS["DOA11"] = 0;
  SPECIAL_NETS["DOA15"] = 0;
  SPECIAL_NETS["DOB3"] = 0;
  SPECIAL_NETS["DOB7"] = 0;
  SPECIAL_NETS["DOB11"] = 0;
  SPECIAL_NETS["DOB15"] = 0;
  SPECIAL_NETS["ADDRA8"] = 0;
  SPECIAL_NETS["ADDRA9"] = 0;
  SPECIAL_NETS["ADDRA10"] = 0;
  SPECIAL_NETS["ADDRA11"] = 0;
  SPECIAL_NETS["ADDRB8"] = 0;
  SPECIAL_NETS["ADDRB9"] = 0;
  SPECIAL_NETS["ADDRB10"] = 0;
  SPECIAL_NETS["ADDRB11"] = 0;
}

void INIT_MAYBE_BIDIRECTIONAL_NETS() {
  MAYBE_BIDIRECTIONAL_NETS["BRAM_RADDR"] = 0;
  MAYBE_BIDIRECTIONAL_NETS["BRAM_RDIN"] = 1;
  MAYBE_BIDIRECTIONAL_NETS["BRAM_RDOUT"] = 2;
  MAYBE_BIDIRECTIONAL_NETS["H6"] = 3;
  MAYBE_BIDIRECTIONAL_NETS["V6"] = 4;
  MAYBE_BIDIRECTIONAL_NETS["TOP_H6"] = 5;
  MAYBE_BIDIRECTIONAL_NETS["TOP_V6"] = 6;
  MAYBE_BIDIRECTIONAL_NETS["BOT_H6"] = 7;
  MAYBE_BIDIRECTIONAL_NETS["BOT_V6"] = 8;
  MAYBE_BIDIRECTIONAL_NETS["LEFT_H6"] = 9;
  MAYBE_BIDIRECTIONAL_NETS["LEFT_V6"] = 10;
  MAYBE_BIDIRECTIONAL_NETS["RIGHT_H6"] = 11;
  MAYBE_BIDIRECTIONAL_NETS["RIGHT_V6"] = 12;
  MAYBE_BIDIRECTIONAL_NETS["UL_H6"] = 13;
  MAYBE_BIDIRECTIONAL_NETS["UL_V6"] = 14;
  MAYBE_BIDIRECTIONAL_NETS["UR_H6"] = 15;
  MAYBE_BIDIRECTIONAL_NETS["UR_V6"] = 16;
  MAYBE_BIDIRECTIONAL_NETS["LL_H6"] = 17;
  MAYBE_BIDIRECTIONAL_NETS["LL_V6"] = 18;
  MAYBE_BIDIRECTIONAL_NETS["LR_H6"] = 19;
  MAYBE_BIDIRECTIONAL_NETS["LR_V6"] = 20;
}

int getWireType(const string &wireName) {
  std::string wireType;

  wireType = wireName.substr(0, 10);
  if (wireType.compare("BRAM_RADDR") == 0)
    return 0;

  wireType = wireName.substr(0, 9);
  if (wireType.compare("BRAM_RDIN") == 0)
    return 1;

  wireType = wireName.substr(0, 10);
  if (wireType.compare("BRAM_RDOUT") == 0)
    return 2;

  wireType = wireName.substr(0, 2);
  if (wireType.compare("H6") == 0)
    return 3;

  wireType = wireName.substr(0, 2);
  if (wireType.compare("V6") == 0)
    return 4;

  wireType = wireName.substr(0, 6);
  if (wireType.compare("TOP_H6") == 0)
    return 5;

  wireType = wireName.substr(0, 6);
  if (wireType.compare("TOP_V6") == 0)
    return 6;

  wireType = wireName.substr(0, 6);
  if (wireType.compare("BOT_H6") == 0)
    return 7;

  wireType = wireName.substr(0, 6);
  if (wireType.compare("BOT_V6") == 0)
    return 8;

  wireType = wireName.substr(0, 7);
  if (wireType.compare("LEFT_H6") == 0)
    return 9;

  wireType = wireName.substr(0, 7);
  if (wireType.compare("LEFT_V6") == 0)
    return 10;

  wireType = wireName.substr(0, 8);
  if (wireType.compare("RIGHT_H6") == 0)
    return 11;

  wireType = wireName.substr(0, 8);
  if (wireType.compare("RIGHT_V6") == 0)
    return 12;

  wireType = wireName.substr(0, 5);
  if (wireType.compare("UL_H6") == 0)
    return 13;

  wireType = wireName.substr(0, 5);
  if (wireType.compare("UL_V6") == 0)
    return 14;

  wireType = wireName.substr(0, 5);
  if (wireType.compare("UR_H6") == 0)
    return 15;

  wireType = wireName.substr(0, 5);
  if (wireType.compare("UR_V6") == 0)
    return 16;

  wireType = wireName.substr(0, 5);
  if (wireType.compare("LL_H6") == 0)
    return 17;

  wireType = wireName.substr(0, 5);
  if (wireType.compare("LL_V6") == 0)
    return 18;

  wireType = wireName.substr(0, 5);
  if (wireType.compare("LR_H6") == 0)
    return 19;

  wireType = wireName.substr(0, 5);
  if (wireType.compare("LR_V6") == 0)
    return 20;

  wireType = wireName.substr(0, 4);
  if (wireType.compare("TBUF") == 0)
    return 21;

  wireType = wireName.substr(0, 9);
  if (wireType.compare("LEFT_TBUF") == 0)
    return 22;

  wireType = wireName.substr(0, 10);
  if (wireType.compare("RIGHT_TBUF") == 0)
    return 23;

  throw CktException("Net: wire " + wireName + " not exist when getWireType");
}

string getOppositeDir(const string &wireDir) {
  if (wireDir.compare("W") == 0)
    return std::string("E");
  if (wireDir.compare("E") == 0)
    return std::string("W");
  if (wireDir.compare("S") == 0)
    return std::string("N");
  if (wireDir.compare("N") == 0)
    return std::string("S");
  if (wireDir.compare("D") == 0)
    return std::string("A");
  if (wireDir.compare("A") == 0)
    return std::string("D");
  if (wireDir.compare("C") == 0)
    return std::string("B");
  if (wireDir.compare("B") == 0)
    return std::string("C");
  if (wireDir.compare("M") == 0)
    return std::string("M");
  throw CktException("Net: unknown direction: " + wireDir);
}

int getBRAMRow(int row) {
  if (row % 4 == 0)
    return row;
  else
    return row + 4 - (row % 4);
}

string getBRAMSeg(int row) {
  if (row % 4 == 0)
    return std::string("D");
  else if (row % 4 == 1)
    return std::string("A");
  else if (row % 4 == 2)
    return std::string("B");
  else
    return std::string("C");
}

string editBRAMTileName(const string &name) {
  if (name.substr(0, 5) == "LBRAM" || name.substr(0, 5) == "RBRAM") {
    if (name[0] == 'L')
      return name.substr(1) + "C0";
    else if (name[0] == 'R')
      return name.substr(1) + "C3";
    else
      throw CktException("xdl: can not edit such tile name ... " + name);
  } else if (name.substr(0, 4) == "BRAM") {
    if (name[name.length() - 1] == '0')
      return "L" + name.substr(0, name.length() - 2);
    else if (name[name.length() - 1] == '3')
      return "R" + name.substr(0, name.length() - 2);
    else
      throw CktException("xdl: can not edit such tile name ... " + name);
  } else
    return name;
}

void addDrivers(const string &cktTileName, const string &wireName, int wireType,
                vector<routeInfo> &drivers) {
  int row, column;
  std::string WireDir, WireNum, OppoDir, DriveName, BrkNym;
  sizeSpan BrkRC;
  std::ostringstream osstrBrkNym;

  string tileName;
  if (args._package == PACKAGE::CB228)
    tileName = editBRAMTileName(cktTileName);
  else
    tileName = cktTileName;

  switch (wireType) {
  case 0:
    WireDir = wireName.substr(10, 1);
    WireNum = wireName.substr(11);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "BRAM_RADDR" + OppoDir + WireNum;
    BrkNym = tileName.substr(4);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    column = BrkRC._columnSpan;
    if (OppoDir.compare("S") == 0)
      row = BrkRC._rowSpan - 4;
    else if (OppoDir.compare("N") == 0)
      row = BrkRC._rowSpan + 4;
    else
      throw CktException("Net: unknown direction of BRAM_RADDR " + tileName +
                         ":" + wireName);
    osstrBrkNym << "BRAMR" << row << "C" << column;
    BrkNym = osstrBrkNym.str();
    break;
  case 1:
    WireDir = wireName.substr(9, 1);
    WireNum = wireName.substr(10);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "BRAM_RDIN" + OppoDir + WireNum;
    BrkNym = tileName.substr(4);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    column = BrkRC._columnSpan;
    if (OppoDir.compare("S") == 0)
      row = BrkRC._rowSpan - 4;
    else if (OppoDir.compare("N") == 0)
      row = BrkRC._rowSpan + 4;
    else
      throw CktException("unknown direction of BRAM_RDIN " + tileName + ":" +
                         wireName);
    osstrBrkNym << "BRAMR" << row << "C" << column;
    BrkNym = osstrBrkNym.str();
    break;
  case 2:
    WireDir = wireName.substr(10, 1);
    WireNum = wireName.substr(11);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "BRAM_RDOUT" + OppoDir + WireNum;
    BrkNym = tileName.substr(4);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    column = BrkRC._columnSpan;
    if (OppoDir.compare("S") == 0)
      row = BrkRC._rowSpan - 4;
    else if (OppoDir.compare("N") == 0)
      row = BrkRC._rowSpan + 4;
    else
      throw CktException("unknown direction of BRAM_RDOUT " + tileName + ":" +
                         wireName);
    osstrBrkNym << "BRAMR" << row << "C" << column;
    BrkNym = osstrBrkNym.str();
    break;
  case 3:
    WireDir = wireName.substr(2, 1);
    WireNum = wireName.substr(3);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "H6" + OppoDir + WireNum;
    BrkNym = tileName;
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    row = BrkRC._rowSpan;
    if (OppoDir.compare("E") == 0) {
      column = BrkRC._columnSpan - 6;
      if (column > 0)
        osstrBrkNym << "R" << row << "C" << column;
      else if (column == 0) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_" + DriveName;
      } else if (column == -1) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_H6A" + WireNum;
      } else if (column == -2) {
        osstrBrkNym << "BRAMR" << getBRAMRow(row) << "C0";
        DriveName = "BRAM_H6B" + getBRAMSeg(row) + WireNum;
        routeInfo driver;
        if (args._package == PACKAGE::CB228)
          driver._siteName = editBRAMTileName(osstrBrkNym.str());
        else
          driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);
        //
        osstrBrkNym.str("");
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_H6B" + WireNum;
      } else if (column == -3) {
        osstrBrkNym << "BRAMR" << getBRAMRow(row) << "C0";
        DriveName = "BRAM_H6M" + getBRAMSeg(row) + WireNum;
        routeInfo driver;
        if (args._package == PACKAGE::CB228)
          driver._siteName = editBRAMTileName(osstrBrkNym.str());
        else
          driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);
        //
        osstrBrkNym.str("");
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_H6M" + WireNum;
      } else if (column == -4) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_H6C" + WireNum;
      } else if (column == -5) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_H6D" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else if (OppoDir.compare("W") == 0) {
      column = BrkRC._columnSpan + 6;
      // 			if(column < 31)
      if (column < INNER_SCALE._columnSpan + 1)
        osstrBrkNym << "R" << row << "C" << column;
      //			else if(column ==31)
      else if (column == INNER_SCALE._columnSpan + 1) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_" + DriveName;
      }
      //			else if(column ==32)
      else if (column == INNER_SCALE._columnSpan + 2) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_H6D" + WireNum;
      }
      //			else if(column ==33)
      else if (column == INNER_SCALE._columnSpan + 3) {
        osstrBrkNym << "BRAMR" << getBRAMRow(row) << "C3";
        DriveName = "BRAM_H6M" + getBRAMSeg(row) + WireNum;
        routeInfo driver;
        if (args._package == PACKAGE::CB228)
          driver._siteName = editBRAMTileName(osstrBrkNym.str());
        else
          driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);
        //
        osstrBrkNym.str("");
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_H6C" + WireNum;
      }
      //			else if(column ==34)
      else if (column == INNER_SCALE._columnSpan + 4) {
        osstrBrkNym << "BRAMR" << getBRAMRow(row) << "C3";
        DriveName = "BRAM_H6B" + getBRAMSeg(row) + WireNum;
        routeInfo driver;
        if (args._package == PACKAGE::CB228)
          driver._siteName = editBRAMTileName(osstrBrkNym.str());
        else
          driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);
        //
        osstrBrkNym.str("");
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_H6M" + WireNum;
      }
      //			else if(column ==35)
      else if (column == INNER_SCALE._columnSpan + 5) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_H6B" + WireNum;
      }
      //			else if(column ==36)
      else if (column == INNER_SCALE._columnSpan + 6) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_H6A" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else
      throw CktException("unknown direction of H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 4:
    WireDir = wireName.substr(2, 1);
    WireNum = wireName.substr(3);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "V6" + OppoDir + WireNum;
    BrkNym = tileName;
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    column = BrkRC._columnSpan;
    if (OppoDir.compare("S") == 0) {
      row = BrkRC._rowSpan - 6;
      if (row > 0)
        osstrBrkNym << "R" << row << "C" << column;
      else if (row == 0) {
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_" + DriveName;
      } else if (row == -1) {
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_V6D" + WireNum;
      } else if (row == -2) {
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_V6C" + WireNum;
      } else if (row == -3) {
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_V6M" + WireNum;
      } else if (row == -4) {
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_V6B" + WireNum;
      } else if (row == -5) {
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_V6A" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else if (OppoDir.compare("N") == 0) {
      row = BrkRC._rowSpan + 6;
      //			if(row < 21)
      if (row < INNER_SCALE._rowSpan + 1)
        osstrBrkNym << "R" << row << "C" << column;
      //			else if(row ==21)
      else if (row == INNER_SCALE._rowSpan + 1) {
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_" + DriveName;
      }
      //			else if(row ==22)
      else if (row == INNER_SCALE._rowSpan + 2) {
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_V6A" + WireNum;
      }
      //			else if(row ==23)
      else if (row == INNER_SCALE._rowSpan + 3) {
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_V6B" + WireNum;
      }
      //			else if(row ==24)
      else if (row == INNER_SCALE._rowSpan + 4) {
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_V6M" + WireNum;
      }
      //			else if(row ==25)
      else if (row == INNER_SCALE._rowSpan + 5) {
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_V6C" + WireNum;
      }
      //			else if(row ==26)
      else if (row == INNER_SCALE._rowSpan + 6) {
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_V6D" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else
      throw CktException("unknown direction of V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 5:
    WireDir = wireName.substr(6, 1);
    WireNum = wireName.substr(7);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "H6" + OppoDir + WireNum;
    BrkNym = "R0" + tileName.substr(1);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    if (OppoDir.compare("E") == 0) {
      column = BrkRC._columnSpan - 6;
      //			if(column > 0 && column!=10 && column!=15){
      // osstrBrkNym<<"TC"<<column;DriveName = "TOP_"+DriveName;}
      if ((args._device == CHIPTYPE::FDP1000K && column > 0 && column != 10 &&
           column != 15) ||
          (args._device == CHIPTYPE::FDP3000K && column > 0 && column != 19 &&
           column != 24)) {
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_" + DriveName;
      } else if (column == 0) {
        osstrBrkNym << "TL";
        DriveName = "UL_" + DriveName;
      } else if (column == -1) {
        osstrBrkNym << "TL";
        DriveName = "UL_H6A" + WireNum;
      } else if (column == -2) {
        osstrBrkNym << "TL";
        DriveName = "UL_H6B" + WireNum;
      } else if (column == -3) {
        osstrBrkNym << "TL";
        DriveName = "UL_H6M" + WireNum;
      } else if (column == -4) {
        osstrBrkNym << "TL";
        DriveName = "UL_H6C" + WireNum;
      } else if (column == -5) {
        osstrBrkNym << "TL";
        DriveName = "UL_H6D" + WireNum;
      }
      //			else if(column== 10)
      else if ((args._device == CHIPTYPE::FDP1000K && column == 10) ||
               (args._device == CHIPTYPE::FDP3000K && column == 19)) {
        osstrBrkNym.str("TM");
        DriveName = "CLKT_H6D" + WireNum;
        routeInfo driver;
        driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);

        osstrBrkNym.str("");
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_H6E" + WireNum;
      }
      //			else if(column== 15)
      else if ((args._device == CHIPTYPE::FDP1000K && column == 15) ||
               (args._device == CHIPTYPE::FDP3000K && column == 24)) {
        osstrBrkNym.str("TM");
        DriveName = "CLKT_H6E" + WireNum;
        routeInfo driver;
        driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);

        osstrBrkNym.str("");
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_H6E" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else if (OppoDir.compare("W") == 0) {
      column = BrkRC._columnSpan + 6;
      //			if(column< 31 && column!=16 && column!=21)
      //			if(column < INNER_SCALE._columnSpan + 1 &&
      // column!=16 && column!=21)
      if ((args._device == CHIPTYPE::FDP1000K &&
           column < INNER_SCALE._columnSpan + 1 && column != 16 &&
           column != 21) ||
          (args._device == CHIPTYPE::FDP3000K &&
           column < INNER_SCALE._columnSpan + 1 && column != 25 &&
           column != 30)) {
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_" + DriveName;
      }
      //			else if(column ==31)
      else if (column == INNER_SCALE._columnSpan + 1) {
        osstrBrkNym << "TR";
        DriveName = "UR_" + DriveName;
      }
      //			else if(column ==32)
      else if (column == INNER_SCALE._columnSpan + 2) {
        osstrBrkNym << "TR";
        DriveName = "UR_H6D" + WireNum;
      }
      //			else if(column ==33)
      else if (column == INNER_SCALE._columnSpan + 3) {
        osstrBrkNym << "TR";
        DriveName = "UR_H6C" + WireNum;
      }
      //			else if(column ==34)
      else if (column == INNER_SCALE._columnSpan + 4) {
        osstrBrkNym << "TR";
        DriveName = "UR_H6M" + WireNum;
      }
      //			else if(column ==35)
      else if (column == INNER_SCALE._columnSpan + 5) {
        osstrBrkNym << "TR";
        DriveName = "UR_H6B" + WireNum;
      }
      //			else if(column ==36)
      else if (column == INNER_SCALE._columnSpan + 6) {
        osstrBrkNym << "TR";
        DriveName = "UR_H6A" + WireNum;
      }
      //			else if(column ==16)
      else if ((args._device == CHIPTYPE::FDP1000K && column == 16) ||
               (args._device == CHIPTYPE::FDP3000K && column == 25)) {
        osstrBrkNym.str("TM");
        DriveName = "CLKT_H6D" + WireNum;
        routeInfo driver;
        driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);

        osstrBrkNym.str("");
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_H6W" + WireNum;
      }
      //			else if(column ==21)
      else if ((args._device == CHIPTYPE::FDP1000K && column == 21) ||
               (args._device == CHIPTYPE::FDP3000K && column == 30)) {
        osstrBrkNym.str("TM");
        DriveName = "CLKT_H6E" + WireNum;
        routeInfo driver;
        driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);

        osstrBrkNym.str("");
        osstrBrkNym << "TC" << column;
        DriveName = "TOP_H6W" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else
      throw CktException("unknown direction of TOP_H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 6:
    WireDir = wireName.substr(6, 1);
    WireNum = wireName.substr(7);
    //		OppoDir = getOppoDir(WireDir);
    //		DriveName = "V6" + OppoDir + WireNum;
    DriveName = "V6N" + WireNum;
    BrkNym = "R0" + tileName.substr(1);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    column = BrkRC._columnSpan;
    /*		if(OppoDir.compare("N") == 0)
                    osstrBrkNym<<"R6C"<<column;*/
    if (WireDir.compare("A") == 0)
      osstrBrkNym << "R1C" << column;
    else if (WireDir.compare("B") == 0)
      osstrBrkNym << "R2C" << column;
    else if (WireDir.compare("M") == 0)
      osstrBrkNym << "R3C" << column;
    else if (WireDir.compare("C") == 0)
      osstrBrkNym << "R4C" << column;
    else if (WireDir.compare("D") == 0)
      osstrBrkNym << "R5C" << column;
    else if (WireDir.compare("S") == 0)
      osstrBrkNym << "R6C" << column;
    else
      throw CktException("unknown direction of TOP_V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 7:
    WireDir = wireName.substr(6, 1);
    WireNum = wireName.substr(7);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "H6" + OppoDir + WireNum;
    /*		BrkNym = "R21"+tileName.substr(1);*/
    BrkNym = "R" + lexical_cast<string>(INNER_SCALE._rowSpan + 1) +
             tileName.substr(1);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    if (OppoDir.compare("E") == 0) {
      column = BrkRC._columnSpan - 6;
      //			if(column > 0 && column!=10 && column!=15)
      if ((args._device == CHIPTYPE::FDP1000K && column > 0 && column != 10 &&
           column != 15) ||
          (args._device == CHIPTYPE::FDP3000K && column > 0 && column != 19 &&
           column != 24)) {
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_" + DriveName;
      } else if (column == 0) {
        osstrBrkNym << "BL";
        DriveName = "LL_" + DriveName;
      } else if (column == -1) {
        osstrBrkNym << "BL";
        DriveName = "LL_H6A" + WireNum;
      } else if (column == -2) {
        osstrBrkNym << "BL";
        DriveName = "LL_H6B" + WireNum;
      } else if (column == -3) {
        osstrBrkNym << "BL";
        DriveName = "LL_H6M" + WireNum;
      } else if (column == -4) {
        osstrBrkNym << "BL";
        DriveName = "LL_H6C" + WireNum;
      } else if (column == -5) {
        osstrBrkNym << "BL";
        DriveName = "LL_H6D" + WireNum;
      }
      //			else if(column== 10)
      else if ((args._device == CHIPTYPE::FDP1000K && column == 10) ||
               (args._device == CHIPTYPE::FDP3000K && column == 19)) {
        osstrBrkNym.str("BM");
        DriveName = "CLKB_H6D" + WireNum;
        routeInfo driver;
        driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);

        osstrBrkNym.str("");
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_H6E" + WireNum;
      }
      //			else if(column== 15)
      else if ((args._device == CHIPTYPE::FDP1000K && column == 15) ||
               (args._device == CHIPTYPE::FDP3000K && column == 24)) {
        osstrBrkNym.str("BM");
        DriveName = "CLKB_H6E" + WireNum;
        routeInfo driver;
        driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);

        osstrBrkNym.str("");
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_H6E" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else if (OppoDir.compare("W") == 0) {
      column = BrkRC._columnSpan + 6;
      //			if(column< 31 && column!=16 && column!=21)
      //			if(column < INNER_SCALE._columnSpan + 1 &&
      // column!=16 && column!=21)
      if ((args._device == CHIPTYPE::FDP1000K &&
           column < INNER_SCALE._columnSpan + 1 && column != 16 &&
           column != 21) ||
          (args._device == CHIPTYPE::FDP3000K &&
           column < INNER_SCALE._columnSpan + 1 && column != 25 &&
           column != 30)) {
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_" + DriveName;
      }
      //			else if(column ==31)
      else if (column == INNER_SCALE._columnSpan + 1) {
        osstrBrkNym << "BR";
        DriveName = "LR_" + DriveName;
      }
      //			else if(column ==32)
      else if (column == INNER_SCALE._columnSpan + 2) {
        osstrBrkNym << "BR";
        DriveName = "LR_H6D" + WireNum;
      }
      //			else if(column ==33)
      else if (column == INNER_SCALE._columnSpan + 3) {
        osstrBrkNym << "BR";
        DriveName = "LR_H6C" + WireNum;
      }
      //			else if(column ==34)
      else if (column == INNER_SCALE._columnSpan + 4) {
        osstrBrkNym << "BR";
        DriveName = "LR_H6M" + WireNum;
      }
      //			else if(column ==35)
      else if (column == INNER_SCALE._columnSpan + 5) {
        osstrBrkNym << "BR";
        DriveName = "LR_H6B" + WireNum;
      }
      //			else if(column ==36)
      else if (column == INNER_SCALE._columnSpan + 6) {
        osstrBrkNym << "BR";
        DriveName = "LR_H6A" + WireNum;
      }
      //			else if(column ==16)
      else if ((args._device == CHIPTYPE::FDP1000K && column == 16) ||
               (args._device == CHIPTYPE::FDP3000K && column == 25)) {
        osstrBrkNym.str("BM");
        DriveName = "CLKB_H6D" + WireNum;
        routeInfo driver;
        driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);

        osstrBrkNym.str("");
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_H6W" + WireNum;
      }
      //			else if(column ==21)
      else if ((args._device == CHIPTYPE::FDP1000K && column == 21) ||
               (args._device == CHIPTYPE::FDP3000K && column == 30)) {
        osstrBrkNym.str("BM");
        DriveName = "CLKB_H6E" + WireNum;
        routeInfo driver;
        driver._siteName = osstrBrkNym.str();
        driver._srcNet = DriveName;
        driver._snkNet = wireName;
        drivers.push_back(driver);

        osstrBrkNym.str("");
        osstrBrkNym << "BC" << column;
        DriveName = "BOT_H6W" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else
      throw CktException("unknown direction of BOT_H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 8:
    WireDir = wireName.substr(6, 1);
    WireNum = wireName.substr(7);
    //		OppoDir = getOppoDir(WireDir);
    //		DriveName = "V6" + OppoDir + WireNum;
    DriveName = "V6S" + WireNum;
    //		BrkNym = "R21"+tileName.substr(1);
    BrkNym = "R" + lexical_cast<string>(INNER_SCALE._rowSpan + 1) +
             tileName.substr(1);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    column = BrkRC._columnSpan;
    /*		if(OppoDir.compare("S") == 0)
                    osstrBrkNym<<"R15C"<<column;*/
    if (WireDir.compare("N") == 0)
      //			osstrBrkNym<<"R15C"<<column;
      osstrBrkNym << "R" << INNER_SCALE._rowSpan - 5 << "C" << column;
    else if (WireDir.compare("A") == 0)
      //			osstrBrkNym<<"R16C"<<column;
      osstrBrkNym << "R" << INNER_SCALE._rowSpan - 4 << "C" << column;
    else if (WireDir.compare("B") == 0)
      //			osstrBrkNym<<"R17C"<<column;
      osstrBrkNym << "R" << INNER_SCALE._rowSpan - 3 << "C" << column;
    else if (WireDir.compare("M") == 0)
      //			osstrBrkNym<<"R18C"<<column;
      osstrBrkNym << "R" << INNER_SCALE._rowSpan - 2 << "C" << column;
    else if (WireDir.compare("C") == 0)
      //			osstrBrkNym<<"R19C"<<column;
      osstrBrkNym << "R" << INNER_SCALE._rowSpan - 1 << "C" << column;
    else if (WireDir.compare("D") == 0)
      //			osstrBrkNym<<"R20C"<<column;
      osstrBrkNym << "R" << INNER_SCALE._rowSpan - 0 << "C" << column;
    else
      throw CktException("unknown direction of BOT_V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 9:
    WireDir = wireName.substr(7, 1);
    WireNum = wireName.substr(8);
    //		OppoDir = getOppoDir(WireDir);
    //		DriveName = "H6" + OppoDir + WireNum;
    DriveName = "H6W" + WireNum;
    BrkNym = tileName.substr(1) + "C0";
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    row = BrkRC._rowSpan;
    //		if(OppoDir.compare("W") == 0)
    //			osstrBrkNym<<"R"<<row<<"C6";
    if (WireDir.compare("E") == 0)
      osstrBrkNym << "R" << row << "C6";
    else if (WireDir.compare("A") == 0)
      osstrBrkNym << "R" << row << "C5";
    else if (WireDir.compare("B") == 0) {
      osstrBrkNym << "BRAMR" << getBRAMRow(row) << "C0";
      DriveName = "BRAM_H6B" + getBRAMSeg(row) + WireNum;
      routeInfo driver;
      if (args._package == PACKAGE::CB228)
        driver._siteName = editBRAMTileName(osstrBrkNym.str());
      else
        driver._siteName = osstrBrkNym.str();
      driver._srcNet = DriveName;
      driver._snkNet = wireName;
      drivers.push_back(driver);
      //
      osstrBrkNym.str("");
      osstrBrkNym << "R" << row << "C4";
      DriveName = "H6W" + WireNum;
    } else if (WireDir.compare("M") == 0) {
      osstrBrkNym << "BRAMR" << getBRAMRow(row) << "C0";
      DriveName = "BRAM_H6M" + getBRAMSeg(row) + WireNum;
      routeInfo driver;
      if (args._package == PACKAGE::CB228)
        driver._siteName = editBRAMTileName(osstrBrkNym.str());
      else
        driver._siteName = osstrBrkNym.str();
      driver._srcNet = DriveName;
      driver._snkNet = wireName;
      drivers.push_back(driver);
      //
      osstrBrkNym.str("");
      osstrBrkNym << "R" << row << "C3";
      DriveName = "H6W" + WireNum;
    } else if (WireDir.compare("C") == 0)
      osstrBrkNym << "R" << row << "C2";
    else if (WireDir.compare("D") == 0)
      osstrBrkNym << "R" << row << "C1";
    else
      throw CktException("unknown direction of LEFT_H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 10:
    WireDir = wireName.substr(7, 1);
    WireNum = wireName.substr(8);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "V6" + OppoDir + WireNum;
    BrkNym = tileName.substr(1) + "C0";
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    if (OppoDir.compare("S") == 0) {
      row = BrkRC._rowSpan - 6;
      if (row > 0) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_" + DriveName;
      } else if (row == 0) {
        osstrBrkNym << "TL";
        DriveName = "UL_" + DriveName;
      } else if (row == -1) {
        osstrBrkNym << "TL";
        DriveName = "UL_V6D" + WireNum;
      } else if (row == -2) {
        osstrBrkNym << "TL";
        DriveName = "UL_V6C" + WireNum;
      } else if (row == -3) {
        osstrBrkNym << "TL";
        DriveName = "UL_V6M" + WireNum;
      } else if (row == -4) {
        osstrBrkNym << "TL";
        DriveName = "UL_V6B" + WireNum;
      } else if (row == -5) {
        osstrBrkNym << "TL";
        DriveName = "UL_V6A" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else if (OppoDir.compare("N") == 0) {
      row = BrkRC._rowSpan + 6;
      //			if(row< 21)
      if (row < INNER_SCALE._rowSpan + 1) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_" + DriveName;
      }
      //			else if(row ==21)
      else if (row == INNER_SCALE._rowSpan + 1) {
        osstrBrkNym << "BL";
        DriveName = "LL_" + DriveName;
      }
      //			else if(row ==22)
      else if (row == INNER_SCALE._rowSpan + 2) {
        osstrBrkNym << "BL";
        DriveName = "LL_V6A" + WireNum;
      }
      //			else if(row ==23)
      else if (row == INNER_SCALE._rowSpan + 3) {
        osstrBrkNym << "BL";
        DriveName = "LL_V6B" + WireNum;
      }
      //			else if(row ==24)
      else if (row == INNER_SCALE._rowSpan + 4) {
        osstrBrkNym << "BL";
        DriveName = "LL_V6M" + WireNum;
      }
      //			else if(row ==25)
      else if (row == INNER_SCALE._rowSpan + 5) {
        osstrBrkNym << "BL";
        DriveName = "LL_V6C" + WireNum;
      }
      //			else if(row ==26)
      else if (row == INNER_SCALE._rowSpan + 6) {
        osstrBrkNym << "BL";
        DriveName = "LL_V6D" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else
      throw CktException("unknown direction of LEFT_V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 11:
    WireDir = wireName.substr(8, 1);
    WireNum = wireName.substr(9);
    //		OppoDir = getOppoDir(WireDir);
    //		DriveName = "H6" + OppoDir + WireNum;
    DriveName = "H6E" + WireNum;
    //		BrkNym = tileName.substr(1)+"C31";
    BrkNym = tileName.substr(1) + "C" +
             lexical_cast<string>(INNER_SCALE._columnSpan + 1);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    row = BrkRC._rowSpan;
    //		if(OppoDir.compare("E") == 0)
    //			osstrBrkNym<<"R"<<row<<"C25";
    if (WireDir.compare("A") == 0)
      //			osstrBrkNym<<"R"<<row<<"C30";
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 0;
    else if (WireDir.compare("B") == 0)
      //			osstrBrkNym<<"R"<<row<<"C29";
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 1;
    else if (WireDir.compare("M") == 0) {
      osstrBrkNym << "BRAMR" << getBRAMRow(row) << "C3"; // why "C3", not "C2"?
      DriveName = "BRAM_H6B" + getBRAMSeg(row) + WireNum;
      routeInfo driver;
      if (args._package == PACKAGE::CB228)
        driver._siteName = editBRAMTileName(osstrBrkNym.str());
      else
        driver._siteName = osstrBrkNym.str();
      driver._srcNet = DriveName;
      driver._snkNet = wireName;
      drivers.push_back(driver);
      //
      osstrBrkNym.str("");
      //			osstrBrkNym<<"R"<<row<<"C28";
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 2;
      DriveName = "H6E" + WireNum;
    } else if (WireDir.compare("C") == 0) {
      osstrBrkNym << "BRAMR" << getBRAMRow(row) << "C3"; // why "C3", not "C2"?
      DriveName = "BRAM_H6M" + getBRAMSeg(row) + WireNum;
      routeInfo driver;
      if (args._package == PACKAGE::CB228)
        driver._siteName = editBRAMTileName(osstrBrkNym.str());
      else
        driver._siteName = osstrBrkNym.str();
      driver._srcNet = DriveName;
      driver._snkNet = wireName;
      drivers.push_back(driver);
      //
      osstrBrkNym.str("");
      //			osstrBrkNym<<"R"<<row<<"C27";
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 3;
      DriveName = "H6E" + WireNum;
    } else if (WireDir.compare("D") == 0)
      //			osstrBrkNym<<"R"<<row<<"C26";
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 4;
    else if (WireDir.compare("W") == 0)
      //			osstrBrkNym<<"R"<<row<<"C25";
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 5;
    else
      throw CktException("unknown direction of RIGHT_H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 12:
    WireDir = wireName.substr(8, 1);
    WireNum = wireName.substr(9);
    OppoDir = getOppositeDir(WireDir);
    DriveName = "V6" + OppoDir + WireNum;
    //		BrkNym = tileName.substr(1)+"C31";
    BrkNym = tileName.substr(1) + "C" +
             lexical_cast<string>(INNER_SCALE._columnSpan + 1);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    if (OppoDir.compare("S") == 0) {
      row = BrkRC._rowSpan - 6;
      if (row > 0) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_" + DriveName;
      } else if (row == 0) {
        osstrBrkNym << "TR";
        DriveName = "UR_" + DriveName;
      } else if (row == -1) {
        osstrBrkNym << "TR";
        DriveName = "UR_V6D" + WireNum;
      } else if (row == -2) {
        osstrBrkNym << "TR";
        DriveName = "UR_V6C" + WireNum;
      } else if (row == -3) {
        osstrBrkNym << "TR";
        DriveName = "UR_V6M" + WireNum;
      } else if (row == -4) {
        osstrBrkNym << "TR";
        DriveName = "UR_V6B" + WireNum;
      } else if (row == -5) {
        osstrBrkNym << "TR";
        DriveName = "UR_V6A" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else if (OppoDir.compare("N") == 0) {
      row = BrkRC._rowSpan + 6;
      //			if(row< 21)
      if (row < INNER_SCALE._rowSpan + 1) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_" + DriveName;
      }
      //			else if(row ==21)
      else if (row == INNER_SCALE._rowSpan + 1) {
        osstrBrkNym << "BR";
        DriveName = "LR_" + DriveName;
      }
      //			else if(row ==22)
      else if (row == INNER_SCALE._rowSpan + 2) {
        osstrBrkNym << "BR";
        DriveName = "LR_V6A" + WireNum;
      }
      //			else if(row ==23)
      else if (row == INNER_SCALE._rowSpan + 3) {
        osstrBrkNym << "BR";
        DriveName = "LR_V6B" + WireNum;
      }
      //			else if(row ==24)
      else if (row == INNER_SCALE._rowSpan + 4) {
        osstrBrkNym << "BR";
        DriveName = "LR_V6M" + WireNum;
      }
      //			else if(row ==25)
      else if (row == INNER_SCALE._rowSpan + 5) {
        osstrBrkNym << "BR";
        DriveName = "LR_V6C" + WireNum;
      }
      //			else if(row ==26)
      else if (row == INNER_SCALE._rowSpan + 6) {
        osstrBrkNym << "BR";
        DriveName = "LR_V6D" + WireNum;
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else
      throw CktException("unknown direction of RIGHT_V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 13:
    WireDir = wireName.substr(5, 1);
    WireNum = wireName.substr(6);
    DriveName = "TOP_H6W" + WireNum;
    if (WireDir.compare("E") == 0)
      osstrBrkNym << "TC6";
    else if (WireDir.compare("A") == 0)
      osstrBrkNym << "TC5";
    else if (WireDir.compare("B") == 0)
      osstrBrkNym << "TC4";
    else if (WireDir.compare("M") == 0)
      osstrBrkNym << "TC3";
    else if (WireDir.compare("C") == 0)
      osstrBrkNym << "TC2";
    else if (WireDir.compare("D") == 0)
      osstrBrkNym << "TC1";
    else
      throw CktException("unknown direction of UL_H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 14:
    WireDir = wireName.substr(5, 1);
    WireNum = wireName.substr(6);
    DriveName = "LEFT_V6N" + WireNum;
    if (WireDir.compare("A") == 0)
      osstrBrkNym << "LR1";
    else if (WireDir.compare("B") == 0)
      osstrBrkNym << "LR2";
    else if (WireDir.compare("M") == 0)
      osstrBrkNym << "LR3";
    else if (WireDir.compare("C") == 0)
      osstrBrkNym << "LR4";
    else if (WireDir.compare("D") == 0)
      osstrBrkNym << "LR5";
    else if (WireDir.compare("S") == 0)
      osstrBrkNym << "LR6";
    else
      throw CktException("unknown direction of UL_V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 15:
    WireDir = wireName.substr(5, 1);
    WireNum = wireName.substr(6);
    DriveName = "TOP_H6E" + WireNum;
    if (WireDir.compare("A") == 0)
      //			osstrBrkNym<<"TC30";
      osstrBrkNym << "TC" << INNER_SCALE._columnSpan - 0;
    else if (WireDir.compare("B") == 0)
      //			osstrBrkNym<<"TC29";
      osstrBrkNym << "TC" << INNER_SCALE._columnSpan - 1;
    else if (WireDir.compare("M") == 0)
      //			osstrBrkNym<<"TC28";
      osstrBrkNym << "TC" << INNER_SCALE._columnSpan - 2;
    else if (WireDir.compare("C") == 0)
      //			osstrBrkNym<<"TC27";
      osstrBrkNym << "TC" << INNER_SCALE._columnSpan - 3;
    else if (WireDir.compare("D") == 0)
      //			osstrBrkNym<<"TC26";
      osstrBrkNym << "TC" << INNER_SCALE._columnSpan - 4;
    else if (WireDir.compare("W") == 0)
      //			osstrBrkNym<<"TC25";
      osstrBrkNym << "TC" << INNER_SCALE._columnSpan - 5;
    else
      throw CktException("unknown direction of UR_H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 16:
    WireDir = wireName.substr(5, 1);
    WireNum = wireName.substr(6);
    DriveName = "RIGHT_V6N" + WireNum;
    if (WireDir.compare("A") == 0)
      osstrBrkNym << "RR1";
    else if (WireDir.compare("B") == 0)
      osstrBrkNym << "RR2";
    else if (WireDir.compare("M") == 0)
      osstrBrkNym << "RR3";
    else if (WireDir.compare("C") == 0)
      osstrBrkNym << "RR4";
    else if (WireDir.compare("D") == 0)
      osstrBrkNym << "RR5";
    else if (WireDir.compare("S") == 0)
      osstrBrkNym << "RR6";
    else
      throw CktException("unknown direction of UR_V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 17:
    WireDir = wireName.substr(5, 1);
    WireNum = wireName.substr(6);
    DriveName = "BOT_H6W" + WireNum;
    if (WireDir.compare("E") == 0)
      osstrBrkNym << "BC6";
    else if (WireDir.compare("A") == 0)
      osstrBrkNym << "BC5";
    else if (WireDir.compare("B") == 0)
      osstrBrkNym << "BC4";
    else if (WireDir.compare("M") == 0)
      osstrBrkNym << "BC3";
    else if (WireDir.compare("C") == 0)
      osstrBrkNym << "BC2";
    else if (WireDir.compare("D") == 0)
      osstrBrkNym << "BC1";
    else
      throw CktException("unknown direction of LL_H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 18:
    WireDir = wireName.substr(5, 1);
    WireNum = wireName.substr(6);
    DriveName = "LEFT_V6S" + WireNum;
    if (WireDir.compare("N") == 0)
      //			osstrBrkNym<<"LR15";
      osstrBrkNym << "LR" << INNER_SCALE._rowSpan - 5;
    else if (WireDir.compare("A") == 0)
      //			osstrBrkNym<<"LR16";
      osstrBrkNym << "LR" << INNER_SCALE._rowSpan - 4;
    else if (WireDir.compare("B") == 0)
      //			osstrBrkNym<<"LR17";
      osstrBrkNym << "LR" << INNER_SCALE._rowSpan - 3;
    else if (WireDir.compare("M") == 0)
      //			osstrBrkNym<<"LR18";
      osstrBrkNym << "LR" << INNER_SCALE._rowSpan - 2;
    else if (WireDir.compare("C") == 0)
      //			osstrBrkNym<<"LR19";
      osstrBrkNym << "LR" << INNER_SCALE._rowSpan - 1;
    else if (WireDir.compare("D") == 0)
      //			osstrBrkNym<<"LR20";
      osstrBrkNym << "LR" << INNER_SCALE._rowSpan - 0;
    else
      throw CktException("unknown direction of LL_V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 19:
    WireDir = wireName.substr(5, 1);
    WireNum = wireName.substr(6);
    DriveName = "BOT_H6E" + WireNum;
    if (WireDir.compare("A") == 0)
      //			osstrBrkNym<<"BC30";
      osstrBrkNym << "BC" << INNER_SCALE._columnSpan - 0;
    else if (WireDir.compare("B") == 0)
      //			osstrBrkNym<<"BC29";
      osstrBrkNym << "BC" << INNER_SCALE._columnSpan - 1;
    else if (WireDir.compare("M") == 0)
      //			osstrBrkNym<<"BC28";
      osstrBrkNym << "BC" << INNER_SCALE._columnSpan - 2;
    else if (WireDir.compare("C") == 0)
      //			osstrBrkNym<<"BC27";
      osstrBrkNym << "BC" << INNER_SCALE._columnSpan - 3;
    else if (WireDir.compare("D") == 0)
      //			osstrBrkNym<<"BC26";
      osstrBrkNym << "BC" << INNER_SCALE._columnSpan - 4;
    else if (WireDir.compare("W") == 0)
      //			osstrBrkNym<<"BC25";
      osstrBrkNym << "BC" << INNER_SCALE._columnSpan - 5;
    else
      throw CktException("unknown direction of LR_H6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 20:
    WireDir = wireName.substr(5, 1);
    WireNum = wireName.substr(6);
    DriveName = "RIGHT_V6S" + WireNum;
    if (WireDir.compare("N") == 0)
      //			osstrBrkNym<<"RR15";
      osstrBrkNym << "RR" << INNER_SCALE._rowSpan - 5;
    else if (WireDir.compare("A") == 0)
      //			osstrBrkNym<<"RR16";
      osstrBrkNym << "RR" << INNER_SCALE._rowSpan - 4;
    else if (WireDir.compare("B") == 0)
      //			osstrBrkNym<<"RR17";
      osstrBrkNym << "RR" << INNER_SCALE._rowSpan - 3;
    else if (WireDir.compare("M") == 0)
      //			osstrBrkNym<<"RR18";
      osstrBrkNym << "RR" << INNER_SCALE._rowSpan - 2;
    else if (WireDir.compare("C") == 0)
      //			osstrBrkNym<<"RR19";
      osstrBrkNym << "RR" << INNER_SCALE._rowSpan - 1;
    else if (WireDir.compare("D") == 0)
      //			osstrBrkNym<<"RR20";
      osstrBrkNym << "RR" << INNER_SCALE._rowSpan - 0;
    else
      throw CktException("unknown direction of LR_V6 " + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 21:
    WireDir = wireName.substr(4, 1);
    BrkNym = tileName;
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    row = BrkRC._rowSpan;
    if (WireDir.compare("3") == 0) {
      column = BrkRC._columnSpan - 4;
      if (column > 0) {
        osstrBrkNym << "R" << row << "C" << column;
        DriveName = "TBUF_STUB3";
      } else if (column == 0) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_TBUF1_STUB";
      } else if (column == -1) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_TBUFO2";
      } else if (column == -2) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_TBUFO3";
      } else if (column == -3) {
        osstrBrkNym << "LR" << row;
        DriveName = "LEFT_TBUFO0";
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else if (WireDir.compare("_") == 0) {
      column = BrkRC._columnSpan + 4;
      //			if(column < 31)
      if (column < INNER_SCALE._columnSpan + 1) {
        osstrBrkNym << "R" << row << "C" << column;
        DriveName = "TBUF3";
      }
      //			else if(column== 31)
      else if (column == INNER_SCALE._columnSpan + 1) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_TBUFO1";
      }
      //			else if(column== 32)
      else if (column == INNER_SCALE._columnSpan + 2) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_TBUFO0";
      }
      //			else if(column== 33)
      else if (column == INNER_SCALE._columnSpan + 3) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_TBUFO3";
      }
      //			else if(column== 34)
      else if (column == INNER_SCALE._columnSpan + 4) {
        osstrBrkNym << "RR" << row;
        DriveName = "RIGHT_TBUFO2";
      } else {
        osstrBrkNym.clear();
        DriveName.clear();
      }
    } else
      throw CktException("unknown direction of TBUF" + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 22:
    WireDir = wireName.substr(10, 1);
    BrkNym = tileName.substr(1) + "C0";
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    row = BrkRC._rowSpan;
    if (WireDir.compare("0") == 0) {
      osstrBrkNym << "R" << row << "C1";
      DriveName = "TBUF3";
    } else if (WireDir.compare("3") == 0) {
      osstrBrkNym << "R" << row << "C2";
      DriveName = "TBUF3";
    } else if (WireDir.compare("2") == 0) {
      osstrBrkNym << "R" << row << "C3";
      DriveName = "TBUF3";
    } else if (WireDir.compare("_") == 0) {
      osstrBrkNym << "R" << row << "C4";
      DriveName = "TBUF3";
    } else
      throw CktException("unknown direction of LEFT_TBUF" + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  case 23:
    WireDir = wireName.substr(11, 1);
    //		BrkNym = tileName.substr(1)+"C31";
    BrkNym = tileName.substr(1) + "C" +
             lexical_cast<string>(INNER_SCALE._columnSpan + 1);
    BrkRC = lexical_cast<sizeSpan>(BrkNym);
    row = BrkRC._rowSpan;
    if (WireDir.compare("1") == 0)
    //			{ osstrBrkNym<<"R"<<row<<"C27";DriveName="TBUF_STUB3";}
    {
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 3;
      DriveName = "TBUF_STUB3";
    } else if (WireDir.compare("0") == 0)
    //			{ osstrBrkNym<<"R"<<row<<"C28";DriveName="TBUF_STUB3";}
    {
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 2;
      DriveName = "TBUF_STUB3";
    } else if (WireDir.compare("3") == 0)
    //			{ osstrBrkNym<<"R"<<row<<"C29";DriveName="TBUF_STUB3";}
    {
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 1;
      DriveName = "TBUF_STUB3";
    } else if (WireDir.compare("2") == 0)
    //			{ osstrBrkNym<<"R"<<row<<"C30";DriveName="TBUF_STUB3";}
    {
      osstrBrkNym << "R" << row << "C" << INNER_SCALE._columnSpan - 0;
      DriveName = "TBUF_STUB3";
    } else
      throw CktException("unknown direction of RIGHT_TBUF" + tileName + ":" +
                         wireName);
    BrkNym = osstrBrkNym.str();
    break;
  default:
    throw CktException("unknown type of wire");
  }

  if (!BrkNym.empty() && !DriveName.empty()) {
    routeInfo driver;
    if (args._package == PACKAGE::CB228)
      driver._siteName = editBRAMTileName(BrkNym);
    else
      driver._siteName = BrkNym;
    driver._srcNet = DriveName;
    driver._snkNet = wireName;
    drivers.push_back(driver);
  }
}