
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

  std::size_t nCalls() const { return nCalls_; }
  std::size_t nAttempts() const { return nViolations_ + nImprove_ + nDegrade_ + nEquiv_; }
  std::size_t nViolations() const { return nViolations_; }
  std::size_t nImprove() const { return nImprove_; }
  std::size_t nDegrade() const { return nDegrade_; }
  std::size_t nEquiv() const { return nEquiv_; }

 protected:
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen) = 0;

  std::vector<Item> extractSequence(const Problem &problem, const Solution &solution) const;
  std::vector<std::vector<Item> > extractRows(const Problem &problem, const Solution &solution) const;
  std::vector<std::vector<Item> > extractCuts(const Problem &problem, const Solution &solution) const;
  std::vector<std::vector<Item> > extractPlates(const Problem &problem, const Solution &solution) const;

  bool sequenceValid(const Problem &problem, const std::vector<Item> &sequence) const;
  void runSequence(const Problem &problem, Solution &solution, const std::vector<Item> &sequence);
  void accept(const Problem &problem, Solution &solution, const Solution &incumbent);

 protected:
  std::size_t nCalls_;
  std::size_t nViolations_;
  std::size_t nImprove_;
  std::size_t nDegrade_;
  std::size_t nEquiv_;

  static const int RETRY = 100;
};

struct Shuffle : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "Shuffle"; }
};

struct StackShuffle : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "StackShuffle"; }
};

struct SizeHeuristicShuffle : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "SizeHeuristicShuffle"; }
};

struct ItemInsert : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "ItemInsert"; }
};

struct RowInsert : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "RowInsert"; }
};

struct CutInsert : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "CutInsert"; }
};

struct PlateInsert : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "PlateInsert"; }
};

struct ItemSwap : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "ItemSwap"; }
};

struct RowSwap : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "RowSwap"; }
};

struct CutSwap : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "CutSwap"; }
};

struct PlateSwap : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "PlateSwap"; }
};

struct AdjacentItemSwap : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "AdjacentItemSwap"; }
};

struct AdjacentRowSwap : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "AdjacentRowSwap"; }
};

struct AdjacentCutSwap : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "AdjacentCutSwap"; }
};

struct AdjacentPlateSwap : Move {
  virtual void apply(const Problem &problem, Solution &solution, std::mt19937 &rgen);
  virtual const char* name() const { return "AdjacentPlateSwap"; }
};


#endif

