#include "DebugHelper.hpp"
#include "Vl2xmlApp.h"
#include <iostream>

using namespace VL2XML_PARSER;

vector<string> stepStack;

int main(int argc, char *argv[]) {
  PUSH_STEP;
  Vl2xmlApp *app = new Vl2xmlApp();

  /************************************************************************/
  /*							Initialization */
  /************************************************************************/
  app->initialize();

  /************************************************************************/
  /*							Process Arguments */
  /************************************************************************/
  if (app->ProcessArgs(argc, argv)) {
    app->exitApp(1);
  }
  /************************************************************************/
  /*							Parse verilog design */
  /************************************************************************/
  if (app->parseDesign()) {
    app->exitApp(1);
  }
  /************************************************************************/
  /*							clarify design
   */
  /************************************************************************/
  if (app->clarifyDesign()) {
    app->exitApp(1);
  }
  /************************************************************************/
  /*							Export verilog design */
  /************************************************************************/
  if (app->doExport()) {
    app->exitApp(1);
  }

  if (app->doReport()) {
    app->exitApp(1);
  }
  /*free the app handler or using the smart pointer*/
  POP_STEP;
  return 0;
}
