
#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "problem.hpp"
#include "solution.hpp"

class Solver {
 public:
  static Solution run(const Problem &problem, int seed=1);
 
 private: 
  Solver(const Problem &problem, int seed);
  void run();

  std::vector<Item> createPlacementOrder() const;

 private:
  const Problem &problem_;
  Solution solution_;
  int seed_;
};

#endif

