#include "PKApp.h"

using namespace PACK;

int main(int argc, char *argv[]) {
  PKApp &app = PKApp::instance();

  app.parse_command(argc, argv);
  app.try_process();

  return 0;
}