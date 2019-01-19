
#ifndef DEFECT_HPP
#define DEFECT_HPP

#include "rectangle.hpp"

struct Defect : public Rectangle {
  int id;
  int plateId;

  Defect(int x, int y, int w, int h) {
    *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
  }
};

#endif

