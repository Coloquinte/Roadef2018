
#include "solver.hpp"
#include "sequence_packer.hpp"
#include "ordering_heuristic.hpp"
#include "solution_checker.hpp"

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
  std::mt19937 rgen(seed_);
  int nShuffles = 10000;
  int nKeeps = 10000;
  for (int i = 0; i < nKeeps; ++i) {
    std::vector<Item> sequence = OrderingHeuristic::orderShuffleStacks(problem_, rgen);
    run(sequence);
  }
  for (int i = 0; i < nShuffles; ++i) {
    std::vector<Item> sequence = OrderingHeuristic::orderKeepStacks(problem_, rgen);
    run(sequence);
  }
}

void Solver::run(const std::vector<Item> &sequence) {
  Solution solution = SequencePacker::run(problem_, sequence);
  bool error = SolutionChecker::check(problem_, solution);
  if (error)
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

