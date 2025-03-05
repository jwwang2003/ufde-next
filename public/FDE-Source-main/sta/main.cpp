#include "sta_app.hpp"

int main(int argc, char *argv[]) {
  FDU::STA::STAApp app;
  app.try_process(argc, argv);
  return 0;
}
