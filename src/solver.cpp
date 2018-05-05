
#include "solver.hpp"
#include "sequence_packer.hpp"
#include "ordering_heuristic.hpp"
#include "solution_checker.hpp"

#include "move.hpp"
#include <iostream>

using namespace std;

Solver::Solver(const Problem &problem, size_t seed, size_t nMoves)
: problem_(problem)
, seed_(seed)
, nMoves_(nMoves) {
}

Solution Solver::run(const Problem &problem, size_t seed, size_t nMoves) {
  Solver solver(problem, seed, nMoves);
  solver.run();
  return solver.solution_;
}

void Solver::run() {
  mt19937 rgen(seed_);
  ShuffleMove shuffler1;
  StackShuffleMove shuffler2;
  SwapMove swapper;
  InsertMove inserter;
  for (size_t i = 0; i < nMoves_; ++i) {
    shuffler1.apply(problem_, solution_, rgen);
    shuffler2.apply(problem_, solution_, rgen);
    swapper.apply(problem_, solution_, rgen);
    inserter.apply(problem_, solution_, rgen);
    if (i%100 == 0) {
      SolutionChecker::report(problem_, solution_);
    }
  }
}

