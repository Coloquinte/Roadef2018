
#ifndef UTILS_HPP
#define UTILS_HPP

#include "params.hpp"

namespace utils {
inline bool fitsMinWaste(int a, int b) {
  return a + Params::minWaste <= b || a == b;
}
}

#endif

