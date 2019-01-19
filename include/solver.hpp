
#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "solver_params.hpp"

#include <memory>
#include <random>
#include <chrono>

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
  static Solution run(const Problem &problem, SolverParams params, const Solution &initial=Solution());
 
 private: 
  Solver(const Problem &problem, SolverParams params, const Solution &initial);
  void init(const Solution &initial);
  void run();
  Move* pickMove();
  Move* pickMove(const std::vector<std::pair<std::unique_ptr<Move>, int> > &moves);

  void step();
  MoveStatus accept(Move &move, const Solution &incumbent);
  void updateStats(Move &move, MoveStatus status, const Solution &incumbent);
  void finalReport() const;

  void addInitializer(std::unique_ptr<Move> &&move, int weight=1);
  void addMove(std::unique_ptr<Move> &&move, int weight=1);

 private:
  const Problem &problem_;
  SolverParams params_;
  std::vector<std::pair<std::unique_ptr<Move>, int> > moves_;
  std::vector<std::pair<std::unique_ptr<Move>, int> > initializers_;

  Solution solution_;
  double bestMapped_;
  double bestDensity_;

  std::vector<std::mt19937> rgens_;
  std::size_t nMoves_;
  std::chrono::time_point<std::chrono::system_clock> startTime_;
  std::chrono::time_point<std::chrono::system_clock> endTime_;

  friend class Move;
};

#endif

