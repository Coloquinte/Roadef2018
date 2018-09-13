
#include "solver.hpp"
#include "sequence_packer.hpp"
#include "ordering_heuristic.hpp"
#include "solution_checker.hpp"

#include "move.hpp"
#include <iostream>
#include <chrono>

using namespace std;

Solution Solver::run(const Problem &problem, SolverParams params, vector<int> initial) {
  Solver solver(problem, params, initial);
  solver.run();
  return solver.solution_;
}

Solver::Solver(const Problem &problem, SolverParams params, vector<int> initial)
: problem_(problem)
, params_(params)
, bestMapped_(0.0)
, bestDensity_(0.0)
, rgen_(params.seed)
, nMoves_(0) {

  // Shuffle everything
  initializers_.emplace_back(make_unique<Shuffle>( 4));
  initializers_.emplace_back(make_unique<Shuffle>(16));
  initializers_.emplace_back(make_unique<Shuffle>(64));

  // Shuffle a range
  moves_.emplace_back(make_unique<Shuffle>(4,  8));
  moves_.emplace_back(make_unique<Shuffle>(4, 16));
  moves_.emplace_back(make_unique<Shuffle>(4, 32));

  // Insertions
  moves_.emplace_back(make_unique<ItemInsert>());
  moves_.emplace_back(make_unique<RowInsert>());
  moves_.emplace_back(make_unique<CutInsert>());
  moves_.emplace_back(make_unique<PlateInsert>());

  // Swaps
  moves_.emplace_back(make_unique<ItemSwap>());
  moves_.emplace_back(make_unique<RowSwap>());
  moves_.emplace_back(make_unique<CutSwap>());
  moves_.emplace_back(make_unique<PlateSwap>());

  // Local swaps
  moves_.emplace_back(make_unique<AdjacentItemSwap>());
  moves_.emplace_back(make_unique<AdjacentRowSwap>());
  moves_.emplace_back(make_unique<AdjacentCutSwap>());
  moves_.emplace_back(make_unique<AdjacentPlateSwap>());

  // Reverse a range
  moves_.emplace_back(make_unique<Mirror>(4));
  moves_.emplace_back(make_unique<Mirror>(8));
  moves_.emplace_back(make_unique<Mirror>(16));

  // Swap two ranges
  moves_.emplace_back(make_unique<RangeSwap>());

  for (const unique_ptr<Move> &m : initializers_) m->solver_ = this;
  for (const unique_ptr<Move> &m : moves_) m->solver_ = this;

  init(initial);
}

void Solver::init(vector<int> initial) {
  if (params_.verbosity >= 2) {
    cout << "Trace:" << endl;
  }

  if (initial.empty()) return;
  solution_ = SequencePacker::run(problem_, initial);

  if (params_.verbosity >= 2) {
    cout << SolutionChecker::evalPercentDensity(problem_, solution_) << "%\t0\tInitial" << endl;
  }
}

void Solver::run() {
  auto start = chrono::system_clock::now();
  nMoves_ = 0;

  for (; nMoves_ < params_.initializationRuns; ++nMoves_) {
    chrono::duration<double> elapsed(chrono::system_clock::now() - start);
    if (elapsed.count() * 0.95 > params_.timeLimit) break;
    pickInitializer()->run();
  }

  for (; nMoves_ < params_.moveLimit; ++nMoves_) {
    chrono::duration<double> elapsed(chrono::system_clock::now() - start);
    if (elapsed.count() * 0.95 > params_.timeLimit) break;
    pickMove()->run();
  }

  if (params_.verbosity >= 2) {
    cout << endl << "MoveName        \tTotal\t-\t=\t+\tErr" << endl;
    for (auto &m : moves_) {
      string name = m->name();
      while (name.size() < 16)
        name.append(" ");

      cout << name << "\t" << m->nCall();
      cout << "\t" << m->nDegradation();
      cout << "\t" << m->nPlateau();
      cout << "\t" << m->nImprovement();
      cout << "\t";

      if (m->nViolation())
        cout << m->nViolation();
      else
        cout << "-";

      cout << endl;
    }
  }
  cout << endl;
}

Move* Solver::pickInitializer() {
  if  (initializers_.empty()) throw std::runtime_error("No initialization move provided.");
  uniform_int_distribution<int> dist(0, initializers_.size()-1);
  return initializers_[dist(rgen_)].get();
}

Move* Solver::pickMove() {
  if  (moves_.empty()) throw std::runtime_error("No move provided.");
  uniform_int_distribution<int> dist(0, moves_.size()-1);
  return moves_[dist(rgen_)].get();
}

