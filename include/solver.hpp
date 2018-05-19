
#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "solver_params.hpp"

#include <memory>
#include <random>

class Move;

class Solver {
 public:
  static Solution run(const Problem &problem, SolverParams params);
 
 private: 
  Solver(const Problem &problem, SolverParams params);
  void run();
  Move* pickMove();

 private:
  const Problem &problem_;
  Solution solution_;
  SolverParams params_;

  std::vector<std::unique_ptr<Move> > moves_;
  std::mt19937 rgen_;
  std::size_t nMoves_;

  friend class Move;
};

#endif

