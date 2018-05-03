
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

  void run(const std::vector<Item> &sequence);

 private:
  const Problem &problem_;
  Solution solution_;
  int seed_;
  double mapped_;
  double density_;
};

#endif

