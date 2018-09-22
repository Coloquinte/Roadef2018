
#include "solver.hpp"
#include "sequence_packer.hpp"
#include "ordering_heuristic.hpp"
#include "solution_checker.hpp"

#include "move.hpp"
#include <iostream>
#include <chrono>
#include <thread>

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
, nMoves_(0) {

  vector<size_t> seeds(params_.nbThreads);
  seed_seq seq { params_.seed };
  seq.generate(seeds.begin(), seeds.end());
  for (std::size_t i = 0; i < params_.nbThreads; ++i) {
    rgens_.push_back(std::mt19937(seeds[i]));
  }

  // Shuffle everything
  initializers_.emplace_back(make_unique<Shuffle>( 1));
  initializers_.emplace_back(make_unique<Shuffle>( 4));
  initializers_.emplace_back(make_unique<Shuffle>(16));
  initializers_.emplace_back(make_unique<Shuffle>(64));

  // Shuffle a range
  moves_.emplace_back(make_unique<Shuffle>(1,  8));
  moves_.emplace_back(make_unique<Shuffle>(1, 16));
  moves_.emplace_back(make_unique<Shuffle>(1, 32));
  moves_.emplace_back(make_unique<Shuffle>(4,  8));
  moves_.emplace_back(make_unique<Shuffle>(4, 16));
  moves_.emplace_back(make_unique<Shuffle>(4, 32));

  // Insertions
  moves_.emplace_back(make_unique<ItemInsert>());
  moves_.emplace_back(make_unique<RowInsert>());
  //moves_.emplace_back(make_unique<CutInsert>());
  //moves_.emplace_back(make_unique<PlateInsert>());

  // Swaps
  moves_.emplace_back(make_unique<ItemSwap>());
  moves_.emplace_back(make_unique<RowSwap>());
  moves_.emplace_back(make_unique<CutSwap>());
  //moves_.emplace_back(make_unique<PlateSwap>());

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
  //moves_.emplace_back(make_unique<RangeSwap>());

  for (const unique_ptr<Move> &m : initializers_) m->solver_ = this;
  for (const unique_ptr<Move> &m : moves_) m->solver_ = this;

  init(initial);
}

void Solver::init(vector<int> initial) {
  if (initial.empty()) return;

  solution_ = SequencePacker::run(problem_, initial);
  bestDensity_ = SolutionChecker::evalPercentDensity(problem_, solution_);
  bestMapped_ = SolutionChecker::evalPercentMapped(problem_, solution_);

  if (params_.verbosity >= 2) {
    if (params_.verbosity >= 3) {
      cout << "Initial solution with " << bestDensity_ << "% density";
      if (bestMapped_ < 99.9) cout << " and only " << bestMapped_ << "% mapped";
      cout << endl;
    }
    else {
      cout << bestDensity_ << "%\t0\tInitial" << endl;
    }
  }
}

void Solver::run() {
  auto start = chrono::system_clock::now();
  nMoves_ = 0;

  while (nMoves_ < params_.moveLimit) {
    chrono::duration<double> elapsed(chrono::system_clock::now() - start);
    if (elapsed.count() * 0.95 > params_.timeLimit) break;
    step();
  }
  
  finalReport();
}

Move* Solver::pickMove() {
  if (nMoves_ < params_.initializationRuns) {
    if  (initializers_.empty()) throw std::runtime_error("No initialization move provided.");
    uniform_int_distribution<int> dist(0, initializers_.size()-1);
    return initializers_[dist(rgens_[0])].get();
  }
  else {
    if  (moves_.empty()) throw std::runtime_error("No move provided.");
    uniform_int_distribution<int> dist(0, moves_.size()-1);
    return moves_[dist(rgens_[0])].get();
  }
}

void Solver::step() {
  vector<Solution> incumbents(params_.nbThreads);

  // Move selection
  vector<Move*> moves(params_.nbThreads);
  for (std::size_t i = 0; i < params_.nbThreads; ++i) {
    moves[i] = pickMove();
  }

  // Parallel evaluation
  auto runner = [&](std::size_t ind) {
    incumbents[ind] = moves[ind]->apply(rgens_[ind]);
  };
  vector<thread> threads;
  for (std::size_t i = 0; i < params_.nbThreads; ++i) {
    threads.push_back(thread(runner, i));
  }
  for (std::size_t i = 0; i < params_.nbThreads; ++i) {
    threads[i].join();
  }

  // Sequential acceptance
  for (std::size_t i = 0; i < params_.nbThreads; ++i, ++nMoves_) {
    MoveStatus status = accept(*moves[i], incumbents[i]);
    updateStats(*moves[i], status);
  }
}

Solver::MoveStatus Solver::accept(Move &move, const Solution &incumbent) {
  if (incumbent.nPlates() == 0) {
    if (params_.verbosity >= 3) {
      cout << "No solution found by " << move.name() << endl;
    }
    return MoveStatus::Failure;
  }

  int violations = SolutionChecker::nViolations(problem_, incumbent);
  if (violations != 0) {
    if (params_.verbosity >= 3) {
      cout << "Invalid incumbent solution obtained by " << move.name() << endl;
    }
    if (params_.failOnViolation) {
      incumbent.report();
      SolutionChecker::report(problem_, incumbent);
      exit(1);
    }
    return MoveStatus::Violation;
  }

  double mapped = SolutionChecker::evalPercentMapped(problem_, incumbent);
  double density = SolutionChecker::evalPercentDensity(problem_, incumbent);
  double prevMapped = bestMapped_;
  double prevDensity = bestDensity_;

  MoveStatus status;
  if (mapped > prevMapped) {
    status = MoveStatus::Improvement;
  }
  else if (mapped < prevMapped) {
    status = MoveStatus::Degradation;
  }
  else if (density > prevDensity) {
    solution_ = incumbent;
    status = MoveStatus::Improvement;
  }
  else if (density < prevDensity) {
    status = MoveStatus::Degradation;
  }
  else {
    status = MoveStatus::Plateau;
  }

  if (params_.verbosity >= 3) {
    if (status == MoveStatus::Improvement)      cout << "Improved";
    else if (status == MoveStatus::Degradation) cout << "Rejected";
    else if (status == MoveStatus::Plateau)     cout << "Accepted";
    cout << " solution: " << density << "% density";
    if (mapped < 99.9) cout << " but only " << mapped << "% mapped";
    cout << " obtained by " << move.name() << endl;

    if (params_.verbosity >= 4) {
      int nCommonPrefix = 0;
      int nCommonSuffix = 0;
      for (; nCommonPrefix < incumbent.nPlates() && nCommonPrefix < solution_.nPlates(); ++nCommonPrefix) {
        if (solution_.plates[nCommonPrefix].sequence() != incumbent.plates[nCommonPrefix].sequence())
          break;
      }
      for (; nCommonSuffix < incumbent.nPlates(); ++nCommonSuffix) {
        if (incumbent.nPlates() != solution_.nPlates())
          break;
        int ind = incumbent.nPlates() - 1 - nCommonSuffix;
        if (solution_.plates[ind].sequence() != incumbent.plates[ind].sequence())
          break;
      }

      cout << nCommonPrefix << " first plates and " << nCommonSuffix << " last plates shared with previous solution" << endl;
    }
  }
  else if (status == MoveStatus::Improvement && params_.verbosity >= 2) {
    cout << density << "%\t" << nMoves_ << "\t" << move.name() << endl;
  }

  if (status != MoveStatus::Degradation) {
    solution_ = incumbent;
    bestMapped_ = mapped;
    bestDensity_ = density;
  }
  
  return status;
}

void Solver::updateStats(Move &move, MoveStatus status) {
  ++move.nCall_;
  switch (status) {
    case MoveStatus::Improvement:
      ++move.nImprovement_;
      break;
    case MoveStatus::Degradation:
      ++move.nDegradation_;
      break;
    case MoveStatus::Plateau:
      ++move.nPlateau_;
      break;
    case MoveStatus::Violation:
      ++move.nViolation_;
      break;
    case MoveStatus::Failure:
      break;
  }
}

void Solver::finalReport() const {
  if (params_.verbosity >= 2) {
    int nEvaluated = 0;
    int nImprovement = 0;
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

      nEvaluated += m->nDegradation() + m->nPlateau() + m->nImprovement();
      nImprovement += m->nImprovement();

      cout << endl;
    }
    cout << endl;
    cout << nMoves_ << " moves attempted for " << nEvaluated << " evaluated and " << nImprovement << " improvements" << endl;
  }
}
