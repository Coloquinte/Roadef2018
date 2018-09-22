
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
  enum class MoveStatus {
    Failure,     // Inexpensive phase failed
    Violation,   // Invalid solution returned
    Degradation,
    Plateau,
    Improvement
  };
  static Solution run(const Problem &problem, SolverParams params, std::vector<int> initial=std::vector<int>());
 
 private: 
  Solver(const Problem &problem, SolverParams params, std::vector<int> initial);
  void init(std::vector<int> initial);
  void run();
  Move* pickMove();

  void step();
  MoveStatus accept(Move &move, const Solution &incumbent);
  void updateStats(Move &move, MoveStatus status);
  void finalReport() const;

 private:
  const Problem &problem_;
  SolverParams params_;
  std::vector<std::unique_ptr<Move> > moves_;
  std::vector<std::unique_ptr<Move> > initializers_;

  Solution solution_;
  double bestMapped_;
  double bestDensity_;

  std::vector<std::mt19937> rgens_;
  std::size_t nMoves_;

  friend class Move;
};

#endif

