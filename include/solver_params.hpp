
#ifndef SOLVER_PARAMS_HPP
#define SOLVER_PARAMS_HPP

#include <cstddef>

struct SolverParams {
  int verbosity;
  std::size_t seed;
  std::size_t moveLimit;
  double timeLimit;
};

#endif

