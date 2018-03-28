
#ifndef PARAMS_HPP
#define PARAMS_HPP

struct Params {
  int nPlates;
  int widthPlates;
  int heightPlates;
  int minXX;
  int maxXX;
  int minYY;
  int minWaste;

  Params() {
    nPlates = 100;
    widthPlates = 6000;
    heightPlates = 3210;
    minXX = 100;
    maxXX = 3500;
    minYY = 100;
    minWaste = 20;
  }
};

#endif

