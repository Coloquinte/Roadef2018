// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef UTILS_HPP
#define UTILS_HPP

#include "params.hpp"

namespace utils {
inline bool fitsMinWaste(int a, int b) {
  return a + Params::minWaste <= b || a == b;
}

inline bool fitsMinWaste(int a, bool tight, int b) {
  if (!tight) return a <= b;
  else return fitsMinWaste(a, b);
}

inline int extendToFit(int a, int b, int minWaste) {
  if (a <= b - minWaste) return a;
  else return b;
}
}

#endif

