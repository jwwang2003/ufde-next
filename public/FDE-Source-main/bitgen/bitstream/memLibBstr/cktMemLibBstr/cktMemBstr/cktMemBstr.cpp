#include "bitstream/memLibBstr/cktMemLibBstr/cktMemBstr/cktMemBstr.h"

namespace BitGen {
namespace bitstream {
using namespace std;

void cktMemBstr::construct() { _used = false; }

void cktMemBstr::addMemContents(const string &contents) {
  _used = true;

  for (int idx = 56; idx >= 0; idx -= 8) {
    string word = contents.substr(idx, 8);
    _bitList.erase(_bitList.begin()); // delete the default content
    _bitList.push_back(word.substr(0, 4) + "_" + word.substr(4, 4));
  }
}

} // namespace bitstream
} // namespace BitGen