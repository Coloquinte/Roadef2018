
#include "solver.hpp"
#include "sequence_packer.hpp"
#include "sequence_merger.hpp"
#include "ordering_heuristic.hpp"
#include "solution_checker.hpp"

#include "move.hpp"
#include "merger_move.hpp"
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
  initializers_.emplace_back(make_unique<Shuffle>(  1));
  //initializers_.emplace_back(make_unique<Shuffle>(  4));
  initializers_.emplace_back(make_unique<Shuffle>( 16));
  initializers_.emplace_back(make_unique<Shuffle>( 64));
  initializers_.emplace_back(make_unique<Shuffle>(128));

  if (params_.enablePacking) {
    // Shuffle a subrange
    moves_.emplace_back(make_unique<Shuffle>(1,  8));
    moves_.emplace_back(make_unique<Shuffle>(1, 16));
    //moves_.emplace_back(make_unique<Shuffle>(1, 32));
    moves_.emplace_back(make_unique<Shuffle>(4,  8));
    moves_.emplace_back(make_unique<Shuffle>(4, 16));
    moves_.emplace_back(make_unique<Shuffle>(4, 32));
    //moves_.emplace_back(make_unique<Shuffle>(8,  8));
    moves_.emplace_back(make_unique<Shuffle>(8, 16));
    moves_.emplace_back(make_unique<Shuffle>(8, 32));

    // Insertions
    moves_.emplace_back(make_unique<ItemInsert>());
    moves_.emplace_back(make_unique<RowInsert>());
    moves_.emplace_back(make_unique<CutInsert>());
    //moves_.emplace_back(make_unique<PlateInsert>());

    // Swaps
    moves_.emplace_back(make_unique<ItemSwap>());
    moves_.emplace_back(make_unique<RowSwap>());
    moves_.emplace_back(make_unique<CutSwap>());
    //moves_.emplace_back(make_unique<PlateSwap>());

    // Local swaps
    //moves_.emplace_back(make_unique<AdjacentItemSwap>());
    moves_.emplace_back(make_unique<AdjacentRowSwap>());
    moves_.emplace_back(make_unique<AdjacentCutSwap>());
    //moves_.emplace_back(make_unique<AdjacentPlateSwap>());

    // Reverse a range
    //moves_.emplace_back(make_unique<Mirror>(4));
    moves_.emplace_back(make_unique<Mirror>(8));
    moves_.emplace_back(make_unique<Mirror>(16));

    // Swap two ranges
    //moves_.emplace_back(make_unique<RangeSwap>());
  }

  if (params_.enableMerging) {
    // Optimal sequence merging
    //moves_.emplace_back(make_unique<MergeRow>());
    //moves_.emplace_back(make_unique<MergeCut>());
    //moves_.emplace_back(make_unique<MergePlate>());
    moves_.emplace_back(make_unique<MergeRandomStacks>());
    moves_.emplace_back(make_unique<MergeOneStack>());
  }

  for (const unique_ptr<Move> &m : initializers_) m->solver_ = this;
  for (const unique_ptr<Move> &m : moves_) m->solver_ = this;

  init(initial);
}

void Solver::init(vector<int> initial) {
  if  (initializers_.empty()) throw std::runtime_error("No initialization move provided.");
  if  (moves_.empty()) throw std::runtime_error("No move provided.");

  if (initial.empty()) return;

  vector<Item> sequence;
  for (int id : initial) {
    sequence.push_back(problem_.items()[id]);
  }

  solution_ = SequencePacker::run(problem_, sequence, params_);
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
  startTime_ = chrono::system_clock::now();
  nMoves_ = 0;

  while (nMoves_ < params_.moveLimit) {
    if (chrono::duration<double>(chrono::system_clock::now() - startTime_).count()
      > 0.98 * params_.timeLimit)
      break;
    step();
  }

  endTime_ = chrono::system_clock::now();
  finalReport();
}

Move* Solver::pickMove() {
  Move* move;
  if (nMoves_ < params_.initializationRuns) {
    uniform_int_distribution<int> dist(0, initializers_.size()-1);
    move = initializers_[dist(rgens_[0])].get();
  }
  else {
    uniform_int_distribution<int> dist(0, moves_.size()-1);
    move = moves_[dist(rgens_[0])].get();
  }

  if (params_.verbosity >= 3) {
    cout << "Selected " << move->name() << endl;
  }
  return move;
}

void Solver::step() {
  size_t parallelEvals = min(params_.nbThreads, params_.moveLimit - nMoves_);
  vector<Solution> incumbents(parallelEvals);

  // Move selection
  vector<Move*> moves(parallelEvals);
  for (std::size_t i = 0; i < parallelEvals; ++i) {
    moves[i] = pickMove();
  }

  // Parallel evaluation
  auto runner = [&](std::size_t ind) {
    incumbents[ind] = moves[ind]->apply(rgens_[ind]);
  };
  vector<thread> threads;
  for (std::size_t i = 0; i < parallelEvals; ++i) {
    threads.push_back(thread(runner, i));
  }
  for (std::size_t i = 0; i < parallelEvals; ++i) {
    threads[i].join();
  }

  // Sequential acceptance
  for (std::size_t i = 0; i < parallelEvals; ++i, ++nMoves_) {
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
      incumbent.write("invalid_solution.csv");
      throw std::runtime_error("A move returned an invalid solution.");
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
      ++move.nFailure_;
      break;
  }
}

void Solver::finalReport() const {
  if (params_.verbosity >= 2) {
    int nEvaluated = 0;
    int nImprovement = 0;
    cout << endl << "MoveName        \tTotal\t-\t=\t+\tFail\tErr" << endl;

    std::vector<Move*> allMoves;
    for (auto &m : initializers_)
      allMoves.push_back(m.get());
    for (auto &m : moves_)
      allMoves.push_back(m.get());
    for (Move *m : allMoves) {
      string name = m->name();
      while (name.size() < 16)
        name.append(" ");

      cout << name << "\t" << m->nCall();
      cout << "\t" << m->nDegradation();
      cout << "\t" << m->nPlateau();
      cout << "\t" << m->nImprovement();

      cout << "\t";
      if (m->nFailure())
        cout << m->nFailure();
      else
        cout << "-";

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
    cout << chrono::duration<double>(endTime_ - startTime_).count() << "s optimization time" << endl;
    cout << endl;
  }
}
