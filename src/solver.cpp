
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
, params_(params)
, rgen_(params.seed)
, nMoves_(0) {
  moves_.emplace_back(make_unique<Shuffle>( 4));
  moves_.emplace_back(make_unique<Shuffle>(16));
  moves_.emplace_back(make_unique<Shuffle>(64));
  moves_.emplace_back(make_unique<Shuffle>(4,  8));
  moves_.emplace_back(make_unique<Shuffle>(4, 16));
  moves_.emplace_back(make_unique<Shuffle>(4, 32));
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

  for (const unique_ptr<Move> &m : moves_) {
    m->solver_ = this;
  }
}

Solution Solver::run(const Problem &problem, SolverParams params) {
  Solver solver(problem, params);
  solver.run();
  return solver.solution_;
}

void Solver::run() {
  auto start = chrono::system_clock::now();

  if (params_.verbosity >= 2) {
    cout << "Trace:" << endl;
  }

  for (nMoves_ = 0; nMoves_ < params_.moveLimit; ++nMoves_) {
    Move *move = pickMove();
    Move::Status status = move->run();

    if (status == Move::Status::Improvement && params_.verbosity >= 2) {
      cout << SolutionChecker::evalPercentDensity(problem_, solution_) << "%\t" << nMoves_ << "\t" << move->name() << endl;
    }

    std::chrono::duration<double> elapsed(chrono::system_clock::now() - start);
    if (elapsed.count() / 60.0 > params_.timeLimit)
      break;
  }

  if (params_.verbosity >= 2) {
    cout << endl << "MoveName\tTotal\tErr\t-\t=\t+" << endl;
    for (auto &m : moves_) {
      string name = m->name();
      while (name.size() < 12)
        name.append(" ");
      cout << name << "\t" << m->nCall() << "\t" << m->nViolation() << "\t" << m->nDegradation() << "\t" << m->nPlateau() << "\t" << m->nImprovement() << endl;
    }
  }
}

Move* Solver::pickMove() {
  uniform_int_distribution<int> dist(0, moves_.size()-1);
  return moves_[dist(rgen_)].get();
}

