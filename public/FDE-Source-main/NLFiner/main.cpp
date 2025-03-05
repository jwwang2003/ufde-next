#include "NLF_app.hpp"

using namespace FDU::NLF;

int main(int argc, char *argv[]) {
  NLFApp app;
  app.parse_command(argc, argv);
  app.try_process();

  return EXIT_SUCCESS;
}