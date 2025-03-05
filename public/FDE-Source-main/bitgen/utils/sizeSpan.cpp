#include "utils/sizeSpan.h"

void sizeSpan::extractCoordinate(const char *coordinate, char xAxis, int &x,
                                 char yAxis, int &y) {
  std::string XYStr(coordinate);
  std::string::size_type xIndex, yIndex;
  xIndex = XYStr.find(xAxis);
  yIndex = XYStr.find(yAxis);
  std::string xValue, yValue;
  if (xIndex < yIndex) {
    xValue = XYStr.substr(1, yIndex - 1);
    yValue = XYStr.substr(yIndex + 1);
  } else {
    xValue = XYStr.substr(1, xIndex - 1);
    yValue = XYStr.substr(xIndex + 1);
  }
  std::istringstream xStream(xValue), yStream(yValue);
  xStream >> x;
  yStream >> y;
}