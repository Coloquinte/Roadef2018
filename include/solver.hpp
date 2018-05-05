
#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "problem.hpp"
#include "solution.hpp"

#include <memory>
#include <random>

class Move;

class Solver {
 public:
  static Solution run(const Problem &problem, std::size_t seed, std::size_t nMoves);
 
 private: 
  Solver(const Problem &problem, std::size_t seed, std::size_t nMoves);
  void run();
  Move& pickMove(std::mt19937 &rgen);

 private:
  const Problem &problem_;
  Solution solution_;
  int seed_;
  std::size_t nMoves_;

  std::vector<std::unique_ptr<Move> > moves_;
};

#endif

