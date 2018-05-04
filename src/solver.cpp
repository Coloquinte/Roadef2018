
#include "solver.hpp"
#include "sequence_packer.hpp"
#include "ordering_heuristic.hpp"
#include "solution_checker.hpp"

#include "move.hpp"
#include <iostream>

using namespace std;

Solver::Solver(const Problem &problem, int seed)
: problem_(problem)
, seed_(seed)
, mapped_(0.0)
, density_(0.0) {
}

Solution Solver::run(const Problem &problem, int seed) {
  Solver solver(problem, seed);
  solver.run();
  return solver.solution_;
}

void Solver::run() {
  mt19937 rgen(seed_);
  ShuffleMove shuffler1;
  StackShuffleMove shuffler2;
  SwapMove swapper;
  InsertMove inserter;
  for (int i = 0; i < 100000; ++i) {
    shuffler1.apply(problem_, solution_, rgen);
    shuffler2.apply(problem_, solution_, rgen);
    swapper.apply(problem_, solution_, rgen);
    inserter.apply(problem_, solution_, rgen);
    if (i%100 == 0) {
      double mapped = SolutionChecker::evalPercentMapped(problem_, solution_);
      double density = SolutionChecker::evalPercentDensity(problem_, solution_);
      cout << "Mapped " << mapped << "% with " << density << "% density " << endl;
    }
  }
}

void Solver::run(const vector<Item> &sequence) {
  Solution solution = SequencePacker::run(problem_, sequence);
  int violations = SolutionChecker::nViolations(problem_, solution);
  if (violations != 0)
    return;
  double mapped = SolutionChecker::evalPercentMapped(problem_, solution);
  double density = SolutionChecker::evalPercentDensity(problem_, solution);
  if (mapped > mapped_
      || (mapped == mapped_ && density > density_)) {
    solution_ = solution;
    mapped_ = mapped;
    density_ = density;
  }
}

