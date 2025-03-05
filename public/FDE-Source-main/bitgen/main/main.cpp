#include "main/BGenApp/BGenApp.h"
#include "main/arguments/Args.h"
#include <ctime>

using namespace std;

int main(int argc, char *argv[]) {
  time_t start, end;
  time(&start);
  srand(start);

  args.tryParse(argc, argv);

  FDU_LOG(INFO) << Info("Design: " + args._netlist);
  FDU_LOG(INFO) << Info("Run bitgen ... ");
  BitGen::BGenApp bitgen;
  bitgen.tryBGen();

  time(&end);
  FDU_LOG(INFO) << Info("Successfully generate the bitstream. Elapsed Time: ") << difftime(end, start)
                << "s";

  return 0;
}
