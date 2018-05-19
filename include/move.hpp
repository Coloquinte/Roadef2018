
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
  Status run(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const =0;
  virtual ~Move() {}

  void updateStats(Status status);

  std::size_t nCall() const { return nCall_; }
  std::size_t nViolation() const { return nViolation_; }
  std::size_t nImprovement() const { return nImprovement_; }
  std::size_t nDegradation() const { return nDegradation_; }
  std::size_t nPlateau() const { return nPlateau_; }

 protected:
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen) = 0;

  std::vector<Item> extractSequence(const Problem &problem, const Solution &solution) const;
  std::vector<std::vector<Item> > extractRows(const Problem &problem, const Solution &solution) const;
  std::vector<std::vector<Item> > extractCuts(const Problem &problem, const Solution &solution) const;
  std::vector<std::vector<Item> > extractPlates(const Problem &problem, const Solution &solution) const;

  bool sequenceValid(const Problem &problem, const std::vector<Item> &sequence) const;
  Status runSequence(const Problem &problem, Solution &solution, const std::vector<Item> &sequence);
  Status accept(const Problem &problem, Solution &solution, const Solution &incumbent);

 protected:
  std::size_t nCall_;
  std::size_t nViolation_;
  std::size_t nImprovement_;
  std::size_t nDegradation_;
  std::size_t nPlateau_;

  static const int RETRY = 10;
};

struct Shuffle : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const;
  Shuffle(int chunkSize) : chunkSize(chunkSize) {}
  int chunkSize;
};

struct ItemInsert : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "ItemInsert"; }
};

struct RowInsert : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "RowInsert"; }
};

struct CutInsert : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "CutInsert"; }
};

struct PlateInsert : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "PlateInsert"; }
};

struct ItemSwap : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "ItemSwap"; }
};

struct RowSwap : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "RowSwap"; }
};

struct CutSwap : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "CutSwap"; }
};

struct PlateSwap : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "PlateSwap"; }
};

struct AdjacentItemSwap : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "AdjacentItemSwap"; }
};

struct AdjacentRowSwap : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "AdjacentRowSwap"; }
};

struct AdjacentCutSwap : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "AdjacentCutSwap"; }
};

struct AdjacentPlateSwap : Move {
  virtual Status apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual std::string name() const { return "AdjacentPlateSwap"; }
};

#endif

