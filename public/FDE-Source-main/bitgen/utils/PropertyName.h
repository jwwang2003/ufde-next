#ifndef PROPERTYNAME_H
#define PROPERTYNAME_H

#include <string>

using std::string;

namespace FDU {

namespace CHIPTYPE {
const string FDP250K = "fdp250k";
const string FDP1000K = "fdp1000k";
const string FDP3000K = "fdp3000k";
const string FDP80K = "FDP80K";
const string FDP500K = "FDP500K";
const string FDP500KIP = "FDP500KIP";
} // namespace CHIPTYPE

namespace PACKAGE {
const string PQ208 = "pq208";
const string CB228 = "cb228";
const string FG256 = "fg256";
} // namespace PACKAGE

namespace ARCHNAME {
const string PRIM = "primitive";
const string BLOCK = "block";
} // namespace ARCHNAME
} // namespace FDU

#endif
