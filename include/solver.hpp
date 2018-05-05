
#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "problem.hpp"
#include "solution.hpp"

class Solver {
 public:
  static Solution run(const Problem &problem, std::size_t seed, std::size_t nMoves);
 
 private: 
  Solver(const Problem &problem, std::size_t seed, std::size_t nMoves);
  void run();

  void run(const std::vector<Item> &sequence);

 private:
  const Problem &problem_;
  Solution solution_;
  int seed_;
  std::size_t nMoves_;
};

#endif

