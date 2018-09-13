
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
  static Solution run(const Problem &problem, SolverParams params, std::vector<int> initial=std::vector<int>());
 
 private: 
  Solver(const Problem &problem, SolverParams params, std::vector<int> initial);
  void init(std::vector<int> initial);
  void run();
  Move* pickMove();
  Move* pickInitializer();

 private:
  const Problem &problem_;
  SolverParams params_;
  std::vector<std::unique_ptr<Move> > moves_;
  std::vector<std::unique_ptr<Move> > initializers_;

  Solution solution_;
  double bestMapped_;
  double bestDensity_;

  std::mt19937 rgen_;
  std::size_t nMoves_;

  friend class Move;
};

#endif

