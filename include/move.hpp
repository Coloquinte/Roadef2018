
#ifndef MOVE_HPP
#define MOVE_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "solver.hpp"

#include <random>

class Move {
 public:
  enum class Status {
    Failure,     // Inexpensive phase failed
    Violation,   // Invalid solution returned
    Degradation,
    Plateau,
    Improvement
  };

 public:
  Move();
  Status run();
  virtual std::string name() const =0;
  virtual ~Move() {}

  void updateStats(Status status);

  std::size_t nCall() const { return nCall_; }
  std::size_t nViolation() const { return nViolation_; }
  std::size_t nImprovement() const { return nImprovement_; }
  std::size_t nDegradation() const { return nDegradation_; }
  std::size_t nPlateau() const { return nPlateau_; }

 protected:
  virtual Status apply() = 0;

  std::vector<Item> extractSequence(const Solution &solution) const;
  std::vector<std::vector<Item> > extractRows(const Solution &solution) const;
  std::vector<std::vector<Item> > extractCuts(const Solution &solution) const;
  std::vector<std::vector<Item> > extractPlates(const Solution &solution) const;

  bool sequenceValid(const std::vector<Item> &sequence) const;
  Status runSequence(const std::vector<Item> &sequence);
  Status accept(const Solution &incumbent);

  const Problem&  problem  () const { return solver_->problem_; }
  const Solution& solution () const { return solver_->solution_; }
  Solution&       solution ()       { return solver_->solution_; }
  std::mt19937&   rgen     ()       { return solver_->rgen_; }

 protected:
  std::size_t nCall_;
  std::size_t nViolation_;
  std::size_t nImprovement_;
  std::size_t nDegradation_;
  std::size_t nPlateau_;

  static const int RETRY = 10;

 public:
  Solver *solver_;
};

struct Shuffle : Move {
  virtual Status apply();
  virtual std::string name() const;
  Shuffle(int chunkSize, int windowSize=0)
    : chunkSize_(chunkSize)
    , windowSize_(windowSize) {
  }
  int chunkSize_;
  int windowSize_;
};

struct ItemInsert : Move {
  virtual Status apply();
  virtual std::string name() const { return "ItemInsert"; }
};

struct RowInsert : Move {
  virtual Status apply();
  virtual std::string name() const { return "RowInsert"; }
};

struct CutInsert : Move {
  virtual Status apply();
  virtual std::string name() const { return "CutInsert"; }
};

struct PlateInsert : Move {
  virtual Status apply();
  virtual std::string name() const { return "PlateInsert"; }
};

struct ItemSwap : Move {
  virtual Status apply();
  virtual std::string name() const { return "ItemSwap"; }
};

struct RowSwap : Move {
  virtual Status apply();
  virtual std::string name() const { return "RowSwap"; }
};

struct CutSwap : Move {
  virtual Status apply();
  virtual std::string name() const { return "CutSwap"; }
};

struct PlateSwap : Move {
  virtual Status apply();
  virtual std::string name() const { return "PlateSwap"; }
};

struct AdjacentItemSwap : Move {
  virtual Status apply();
  virtual std::string name() const { return "AdjacentItemSwap"; }
};

struct AdjacentRowSwap : Move {
  virtual Status apply();
  virtual std::string name() const { return "AdjacentRowSwap"; }
};

struct AdjacentCutSwap : Move {
  virtual Status apply();
  virtual std::string name() const { return "AdjacentCutSwap"; }
};

struct AdjacentPlateSwap : Move {
  virtual Status apply();
  virtual std::string name() const { return "AdjacentPlateSwap"; }
};

#endif

