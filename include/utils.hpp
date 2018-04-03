
#ifndef UTILS_HPP
#define UTILS_HPP

#include "problem.hpp"
#include "solution.hpp"

Problem downscale(const Problem &problem, int pitch);
Solution upscale(const Problem &problem, const Solution &solution, int pitch);

#endif

