
#ifndef MOVE_HPP
#define MOVE_HPP

#include "problem.hpp"
#include "solution.hpp"

#include <random>

class Move {
 public:
  Move();
  void run(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const =0;
  virtual ~Move() {}

 protected:
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen) = 0;

  std::vector<Item> extractSequence(const Problem &problem, const Solution &solution) const;
  void runSequence(const Problem &problem, Solution &solution, const std::vector<Item> &sequence);
  bool sequenceValid(const Problem &problem, const std::vector<Item> &sequence) const;
  void accept(const Problem &problem, Solution &solution, const Solution &incumbent);

 private:
  std::size_t nCalls_;
  std::size_t nViolations_;
  std::size_t nImprove_;
  std::size_t nDegrade_;
  std::size_t nEquiv_;
};

struct ShuffleMove : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Shuffle"; }
};

struct StackShuffleMove : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Stack shuffle"; }
};

struct SwapMove : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Swap"; }
};

struct AdjacentSwapMove : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Adjacent swap"; }
};

struct InsertMove : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Insert"; }
};

struct RowInsertMove : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Row insert"; }
};

struct CutInsertMove : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Cut insert"; }
};

struct PlateInsertMove : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Plate insert"; }
};

#endif

