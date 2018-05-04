
#ifndef MOVE_HPP
#define MOVE_HPP

#include "problem.hpp"
#include "solution.hpp"

#include <random>

class Move {
 public:
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen) = 0;

 protected:
  std::vector<Item> extractSequence(const Problem &problem, const Solution &solution) const;
  bool sequenceValid(const Problem &problem, const std::vector<Item> &sequence) const;
  void accept(const Problem &problem, Solution &solution, const Solution &incumbent);
};

class ShuffleMove : public Move {
 public:
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
};

class StackShuffleMove : public Move {
 public:
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
};

class SwapMove : public Move {
 public:
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
};

class InsertMove : public Move {
 public:
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
};

#endif

