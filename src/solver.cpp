
#include "solver.hpp"
#include "sequence_packer.hpp"

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
  solution_ = SequencePacker::run(problem_, sequence);
}

std::vector<Item> Solver::createPlacementOrder() const {
  std::vector<Item> ret;
  for (auto seq : problem_.sequenceItems())
    for (Item item : seq)
      ret.push_back(item);
  return ret;
}

