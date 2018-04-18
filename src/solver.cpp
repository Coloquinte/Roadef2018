
#include "solver.hpp"
#include "sequence_packer.hpp"

using namespace std;

Solver::Solver(const Problem &problem)
: problem_(problem) {
}

Solution Solver::run(const Problem &problem) {
  Solver solver(problem);
  solver.run();
  return solver.solution_;
}

void Solver::run() {
  std::vector<Item> sequence = createPlacementOrder();
  solution_ = SequencePacker::run(problem_, sequence, 50, 50);
}

std::vector<Item> Solver::createPlacementOrder() const {
  std::vector<Item> ret;
  for (auto seq : problem_.stackItems())
    for (Item item : seq)
      ret.push_back(item);

  return ret;
}

