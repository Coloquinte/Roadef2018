
#include "solver.hpp"
#include "sequence_packer.hpp"
#include "ordering_heuristic.hpp"
#include "solution_checker.hpp"

#include "move.hpp"
#include <iostream>
#include <chrono>

using namespace std;

Solver::Solver(const Problem &problem, SolverParams params)
: problem_(problem)
, params_(params) {
  moves_.emplace_back(make_unique<Shuffle>(1));
  moves_.emplace_back(make_unique<Shuffle>(2));
  moves_.emplace_back(make_unique<Shuffle>(4));
  moves_.emplace_back(make_unique<Shuffle>(8));
  moves_.emplace_back(make_unique<Shuffle>(16));
  moves_.emplace_back(make_unique<Shuffle>(32));
  //moves_.emplace_back(make_unique<ItemInsert>());
  //moves_.emplace_back(make_unique<RowInsert>());
  //moves_.emplace_back(make_unique<CutInsert>());
  //moves_.emplace_back(make_unique<PlateInsert>());
  //moves_.emplace_back(make_unique<ItemSwap>());
  //moves_.emplace_back(make_unique<RowSwap>());
  //moves_.emplace_back(make_unique<CutSwap>());
  //moves_.emplace_back(make_unique<PlateSwap>());
  //moves_.emplace_back(make_unique<AdjacentItemSwap>());
  //moves_.emplace_back(make_unique<AdjacentRowSwap>());
  //moves_.emplace_back(make_unique<AdjacentCutSwap>());
  //moves_.emplace_back(make_unique<AdjacentPlateSwap>());
}

Solution Solver::run(const Problem &problem, SolverParams params) {
  Solver solver(problem, params);
  solver.run();
  return solver.solution_;
}

void Solver::run() {
  auto start = chrono::system_clock::now();

  mt19937 rgen(params_.seed);
  for (size_t i = 0; i < params_.moveLimit; ++i) {
    pickMove(rgen).run(problem_, solution_, rgen);

    std::chrono::duration<double> elapsed(chrono::system_clock::now() - start);
    if (elapsed.count() / 60.0 > params_.timeLimit)
      break;
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

