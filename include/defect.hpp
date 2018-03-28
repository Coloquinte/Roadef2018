
#ifndef DEFECT_HPP
#define DEFECT_HPP

#include "rectangle.hpp"

struct Defect : public Rectangle {
  int id;
  int plate_id;

  Defect(int x, int y, int w, int h)
    : Rectangle(x, y, w, h) {
    }
};

#endif

