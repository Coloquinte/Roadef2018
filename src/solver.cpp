
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
  moves_.emplace_back(make_unique<Shuffle>());
  moves_.emplace_back(make_unique<StackShuffle>());
  moves_.emplace_back(make_unique<SizeHeuristicShuffle>());
  moves_.emplace_back(make_unique<ItemInsert>());
  moves_.emplace_back(make_unique<RowInsert>());
  moves_.emplace_back(make_unique<CutInsert>());
  moves_.emplace_back(make_unique<PlateInsert>());
  moves_.emplace_back(make_unique<ItemSwap>());
  moves_.emplace_back(make_unique<RowSwap>());
  moves_.emplace_back(make_unique<CutSwap>());
  moves_.emplace_back(make_unique<PlateSwap>());
  moves_.emplace_back(make_unique<AdjacentItemSwap>());
  moves_.emplace_back(make_unique<AdjacentRowSwap>());
  moves_.emplace_back(make_unique<AdjacentCutSwap>());
  moves_.emplace_back(make_unique<AdjacentPlateSwap>());
}

Solution Solver::run(const Problem &problem, size_t seed, size_t nMoves) {
  Solver solver(problem, seed, nMoves);
  solver.run();
  return solver.solution_;
}

void Solver::run() {
  mt19937 rgen(seed_);
  for (size_t i = 0; i < nMoves_; ++i) {
    pickMove(rgen).run(problem_, solution_, rgen);
  }

  cout << endl << "MoveName\tTotal\tErr\t-\t=\t+" << endl;
  for (auto &m : moves_) {
    string name = m->name();
    while (name.size() < 12)
      name.append(" ");
    cout << name << "\t" << m->nCalls() << "\t" << m->nViolations() << "\t" << m->nDegrade() << "\t" << m->nEquiv() << "\t" << m->nImprove() << endl;
  }
}

Move& Solver::pickMove(mt19937 &rgen) {
  uniform_int_distribution<int> dist(0, moves_.size()-1);
  return *moves_[dist(rgen)];
}

