
#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "problem.hpp"
#include "solution.hpp"

class Solver {
 public:
  static Solution run(const Problem &problem);
 
 private: 
  Solver(const Problem &problem);
  void run();

  std::vector<Item> createPlacementOrder() const;

 private:
  const Problem &problem_;
  Solution solution_;
};

#endif

