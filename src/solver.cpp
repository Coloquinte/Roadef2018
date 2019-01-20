// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#include "solver.hpp"
#include "sequence_packer.hpp"
#include "sequence_merger.hpp"
#include "ordering_heuristic.hpp"
#include "solution_checker.hpp"

#include "move.hpp"
#include "merger_move.hpp"
#include "packer_move.hpp"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

Solution Solver::run(const Problem &problem, SolverParams params, const Solution &initial) {
  Solver solver(problem, params, initial);
  solver.run();
  return solver.solution_;
}

Solver::Solver(const Problem &problem, SolverParams params, const Solution &initial)
: problem_(problem)
, params_(params)
, bestMapped_(0.0)
, bestDensity_(0.0)
, nMoves_(0) {

  vector<size_t> seeds(params_.nbThreads);
  seed_seq seq { params_.seed };
  seq.generate(seeds.begin(), seeds.end());
  for (size_t i = 0; i < params_.nbThreads; ++i) {
    rgens_.push_back(mt19937(seeds[i]));
  }

  // Shuffle everything
  addInitializer(make_unique<Shuffle>(  1));
  //addInitializer(make_unique<Shuffle>(  4));
  addInitializer(make_unique<Shuffle>( 16));
  addInitializer(make_unique<Shuffle>( 64));
  addInitializer(make_unique<Shuffle>(128));

  if (params_.enablePacking) {
    // Shuffle a subrange
    addMove(make_unique<Shuffle>(1,  8));
    addMove(make_unique<Shuffle>(1, 16));
    //addMove(make_unique<Shuffle>(1, 32));
    addMove(make_unique<Shuffle>(4,  8));
    addMove(make_unique<Shuffle>(4, 16));
    addMove(make_unique<Shuffle>(4, 32));
    //addMove(make_unique<Shuffle>(8,  8));
    addMove(make_unique<Shuffle>(8, 16));
    addMove(make_unique<Shuffle>(8, 32));

    // Insertions
    addMove(make_unique<ItemInsert>());
    addMove(make_unique<RowInsert>());
    addMove(make_unique<CutInsert>());
    //addMove(make_unique<PlateInsert>());

    // Swaps
    addMove(make_unique<ItemSwap>());
    addMove(make_unique<RowSwap>());
    addMove(make_unique<CutSwap>());
    //addMove(make_unique<PlateSwap>());

    // Local swaps
    //addMove(make_unique<AdjacentItemSwap>());
    addMove(make_unique<AdjacentRowSwap>());
    addMove(make_unique<AdjacentCutSwap>());
    //addMove(make_unique<AdjacentPlateSwap>());

    // Reverse a range
    //addMove(make_unique<Mirror>(4));
    addMove(make_unique<Mirror>(8));
    addMove(make_unique<Mirror>(16));

    // Swap two ranges
    //addMove(make_unique<RangeSwap>());
    // Local improvement
    addMove(make_unique<PackRowInsert>()    );
    addMove(make_unique<PackCutInsert>()    );
    addMove(make_unique<PackPlateInsert>()  );
    addMove(make_unique<PackRowShuffle>()   );
    addMove(make_unique<PackCutShuffle>()   );
    addMove(make_unique<PackPlateShuffle>() );
  }

  if (params_.enableMerging) {
    // Optimal sequence merging
    //addMove(make_unique<MergeRow>());
    //addMove(make_unique<MergeCut>());
    //addMove(make_unique<MergePlate>());
    addMove(make_unique<MergeRandomStacks>());
    addMove(make_unique<MergeOneStack>());
  }

  for (const auto &m : initializers_) m.first->solver_ = this;
  for (const auto &m : moves_) m.first->solver_ = this;

  init(initial);
}

void Solver::addInitializer(unique_ptr<Move> &&mv, int weight) {
  initializers_.emplace_back(move(mv), weight);
}

void Solver::addMove(unique_ptr<Move> &&mv, int weight) {
  moves_.emplace_back(move(mv), weight);
}

void Solver::init(const Solution &solution) {
  if  (initializers_.empty()) throw runtime_error("No initialization move provided.");
  if  (moves_.empty()) throw runtime_error("No move provided.");

  if (solution.nItems() == 0) return;

  solution_ = solution;
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
    return pickMove(initializers_);
  }
  else {
    return pickMove(moves_);
  }

  if (params_.verbosity >= 3) {
    cout << "Selected " << move->name() << endl;
  }
  return move;
}

Move* Solver::pickMove(const vector<pair<unique_ptr<Move>, int> > &moves) {
  int totWeight = 0;
  for (const auto& m : moves) totWeight += m.second;

  uniform_int_distribution<int> dist(0, totWeight-1);
  int roll = dist(rgens_[0]);

  int weight = 0;
  for (const auto& m : moves) {
    weight += m.second;
    if (weight > roll)
      return m.first.get();
  }
  return moves[0].first.get();
}

void Solver::step() {
  size_t parallelEvals = min(params_.nbThreads, params_.moveLimit - nMoves_);
  vector<Solution> incumbents(parallelEvals);

  // Move selection
  vector<Move*> moves(parallelEvals);
  for (size_t i = 0; i < parallelEvals; ++i) {
    moves[i] = pickMove();
  }

  // Parallel evaluation
  auto runner = [&](size_t ind) {
    incumbents[ind] = moves[ind]->apply(rgens_[ind]);
  };
  vector<thread> threads;
  for (size_t i = 0; i < parallelEvals; ++i) {
    threads.push_back(thread(runner, i));
  }
  for (size_t i = 0; i < parallelEvals; ++i) {
    threads[i].join();
  }

  // Sequential acceptance
  for (size_t i = 0; i < parallelEvals; ++i, ++nMoves_) {
    MoveStatus status = accept(*moves[i], incumbents[i]);
    updateStats(*moves[i], status, incumbents[i]);
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
      throw runtime_error("A move returned an invalid solution.");
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

void Solver::updateStats(Move &move, MoveStatus status, const Solution &incumbent) {
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

  int nCommonPrefix = 0;
  int nCommonSuffix = 0;
  for (; nCommonPrefix < incumbent.nPlates() && nCommonPrefix < solution_.nPlates(); ++nCommonPrefix) {
    if (solution_.plates[nCommonPrefix].sequence() != incumbent.plates[nCommonPrefix].sequence())
      break;
  }
  for (; nCommonSuffix < incumbent.nPlates() - nCommonPrefix; ++nCommonSuffix) {
    if (incumbent.nPlates() != solution_.nPlates())
      break;
    int ind = incumbent.nPlates() - 1 - nCommonSuffix;
    if (solution_.plates[ind].sequence() != incumbent.plates[ind].sequence())
      break;
  }

  move.nCommonPrefixPlates_ += nCommonPrefix;
  move.nCommonSuffixPlates_ += nCommonSuffix;
  move.nDifferentPlates_ += incumbent.nPlates() - nCommonSuffix - nCommonPrefix;
}

void Solver::finalReport() const {
  if (params_.verbosity >= 2) {
    int nEvaluated = 0;
    int nImprovement = 0;
    cout << endl << "MoveName        \tTotal\t-\t=\t+\tFail\tErr\tRecompute" << endl;

    vector<Move*> allMoves;
    for (auto &m : initializers_)
      allMoves.push_back(m.first.get());
    for (auto &m : moves_)
      allMoves.push_back(m.first.get());
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

      cout << "\t";
      if (m->nAcceptable())
        cout << m->recomputationPercentage() << "%";
      else
        cout << "-";

      cout << endl;
    }
    cout << endl;
    cout << nMoves_ << " moves attempted for " << nEvaluated << " evaluated and " << nImprovement << " improvements" << endl;
    cout << chrono::duration<double>(endTime_ - startTime_).count() << "s optimization time" << endl;
    cout << endl;
  }
}
