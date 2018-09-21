
#ifndef SOLVER_PARAMS_HPP
#define SOLVER_PARAMS_HPP

#include <cstddef>

struct SolverParams {
  int verbosity;
  std::size_t seed;
  std::size_t nbThreads;
  std::size_t initializationRuns;
  std::size_t moveLimit;
  double timeLimit;
  bool failOnViolation;

  SolverParams() {
    verbosity = 0;
    seed = 0;
    nbThreads = 0;
    initializationRuns = 0;
    moveLimit = 0;
    timeLimit = 0.0;
    failOnViolation = false;
  }
};

#endif

