
#include "solver.hpp"
#include "sequence_packer.hpp"
#include "ordering_heuristic.hpp"

using namespace std;

Solver::Solver(const Problem &problem, int seed)
: problem_(problem)
, seed_(seed) {
}

Solution Solver::run(const Problem &problem, int seed) {
  Solver solver(problem, seed);
  solver.run();
  return solver.solution_;
}

void Solver::run() {
  std::vector<Item> sequence = OrderingHeuristic::orderShuffleStacks(problem_, seed_);
  solution_ = SequencePacker::run(problem_, sequence);
}

