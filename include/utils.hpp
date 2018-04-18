
#ifndef UTILS_HPP
#define UTILS_HPP

#include "problem.hpp"
#include "solution.hpp"

Problem downscale(const Problem &problem, int pitch);
Solution upscale(const Problem &problem, const Solution &solution, int pitch);

inline int divRoundUp   (int a, int b) { return (a + b - 1) / b; }
inline int divRoundDown (int a, int b) { return a / b; }

#endif

