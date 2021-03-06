// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef SOLVER_PARAMS_HPP
#define SOLVER_PARAMS_HPP

#include <cstddef>

enum class PackingOption {
  Approximate,
  Exact,
  Diagnose
};

struct SolverParams {
  int verbosity;
  std::size_t seed;
  std::size_t nbThreads;
  std::size_t initializationRuns;
  std::size_t moveLimit;
  double timeLimit;
  bool failOnViolation;
  bool earlyCancel;

  PackingOption rowPacking;
  PackingOption cutPacking;
  PackingOption platePacking;
  bool tracePackingFronts;

  SolverParams() {
    verbosity = 0;
    seed = 0;
    nbThreads = 0;
    initializationRuns = 0;
    moveLimit = 0;
    timeLimit = 0.0;
    failOnViolation = false;
    earlyCancel = false;

    rowPacking = PackingOption::Approximate;
    cutPacking = PackingOption::Approximate;
    platePacking = PackingOption::Approximate;
    tracePackingFronts = false;
  }
};

#endif

