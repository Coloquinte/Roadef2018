
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
}

#endif

