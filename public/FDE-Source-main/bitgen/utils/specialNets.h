#ifndef _SPECIALNETS_H_
#define _SPECIALNETS_H_

#include <map>
#include <string>
#include <vector>

#include "utils/cfgInTile.h"

using std::map;
using std::string;
using std::vector;

extern sizeSpan INNER_SCALE;
void INIT_INNER_SCALE();

extern map<string, int> SPECIAL_NETS;
void INIT_SPECIAL_NETS();

extern map<string, int> MAYBE_BIDIRECTIONAL_NETS;
void INIT_MAYBE_BIDIRECTIONAL_NETS();

// used for pip as "=-"
int getWireType(const string &wireName);
string getOppositeDir(const string &wireDir);
int getBRAMRow(int row);
string getBRAMSeg(int row);
void addDrivers(const string &cktTileName, const string &wireName, int wireType,
                vector<routeInfo> &drivers);

// used for differentiate fqvr300 & fdp3000k
string editBRAMTileName(const string &name);

#endif