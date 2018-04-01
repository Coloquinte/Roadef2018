
#ifndef SOLUTION_CHECKER_HPP
#define SOLUTION_CHECKER_HPP

#include "solution.hpp"
#include "problem.hpp"

class SolutionChecker {
 public:
  static void check(const Problem &problem, const Solution &solution);
  static long long evalAreaViolation(const Problem &problem, const Solution &solution);
  static long long evalAreaUsage(const Problem &problem, const Solution &solution);

 private:
  SolutionChecker(const Problem &problem, const Solution &solution);
  void check();
  long long evalAreaViolation();
  long long evalAreaUsage();

  void checkPlate(const PlateSolution &plate);
  void checkCut(const CutSolution &cut);
  void checkRow(const RowSolution &row);
  void checkItem(const ItemSolution &row);

  void checkPlateDivision(const PlateSolution &plate);
  void checkCutDivision(const CutSolution &cut);
  void checkRowDivision(const RowSolution &row);

  void checkItemUnicity();
  void checkSequences();

  bool fitsMinWaste(int a, int b) const;

 private:
  const Problem &problem_;
  const Solution &solution_;
};

#endif

