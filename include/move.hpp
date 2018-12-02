
#ifndef MOVE_HPP
#define MOVE_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "solver.hpp"

#include <random>

class Move {
 public:
  Move();
  virtual Solution apply(std::mt19937& rgen) = 0;
  virtual std::string name() const =0;
  virtual ~Move() {}

  std::size_t nCall() const { return nCall_; }
  std::size_t nViolation() const { return nViolation_; }
  std::size_t nImprovement() const { return nImprovement_; }
  std::size_t nDegradation() const { return nDegradation_; }
  std::size_t nPlateau() const { return nPlateau_; }

 protected:
  std::vector<Item> extractSequence(const Solution &solution) const;
  std::vector<std::vector<Item> > extractItems(const Solution &solution) const;
  std::vector<std::vector<Item> > extractRows(const Solution &solution) const;
  std::vector<std::vector<Item> > extractCuts(const Solution &solution) const;
  std::vector<std::vector<Item> > extractPlates(const Solution &solution) const;

  Solution mergeRepairRun(const std::vector<std::vector<Item> > &sequence);
  Solution runSequence(const std::vector<Item> &sequence);

  std::vector<Item> mergeSequence(const std::vector<std::vector<Item> > &sequence);
  void repairSequence(std::vector<Item> &sequence) const;
  bool sequenceValid(const std::vector<Item> &sequence) const;
  Solution accept(const Solution &incumbent);

  const Problem& problem() const { return solver_->problem_; }
  const Solution& solution() const { return solver_->solution_; }
  const SolverParams& params() const { return solver_->params_; }

  double bestMapped  () { return solver_->bestMapped_; }
  double bestDensity () { return solver_->bestDensity_; }


 protected:
  std::size_t nCall_;
  std::size_t nViolation_;
  std::size_t nImprovement_;
  std::size_t nDegradation_;
  std::size_t nPlateau_;

 public:
  const Solver *solver_;

  friend class Solver;
};

struct Shuffle : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const;
  Shuffle(int chunkSize, int windowSize=0)
    : chunkSize_(chunkSize)
    , windowSize_(windowSize) {
  }
  int chunkSize_;
  int windowSize_;
};

struct ItemInsert : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "ItemInsert"; }
};

struct RowInsert : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "RowInsert"; }
};

struct CutInsert : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "CutInsert"; }
};

struct PlateInsert : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "PlateInsert"; }
};

struct ItemSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "ItemSwap"; }
};

struct RowSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "RowSwap"; }
};

struct CutSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "CutSwap"; }
};

struct PlateSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "PlateSwap"; }
};

struct RangeSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "RangeSwap"; }
};

struct AdjacentItemSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "AdjacentItemSwap"; }
};

struct AdjacentRowSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "AdjacentRowSwap"; }
};

struct AdjacentCutSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "AdjacentCutSwap"; }
};

struct AdjacentPlateSwap : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "AdjacentPlateSwap"; }
};

struct Mirror : Move {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const;
  Mirror(int width)
    : width_(width) {
  }
  int width_;
};

#endif

