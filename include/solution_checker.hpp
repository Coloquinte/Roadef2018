
#ifndef SOLUTION_CHECKER_HPP
#define SOLUTION_CHECKER_HPP

#include "solution.hpp"
#include "problem.hpp"

#include <string>
#include <memory>
#include <unordered_map>

class SolutionChecker {
 public:
  static void report(const Problem &problem, const Solution &solution);
  static int nViolations(const Problem &problem, const Solution &solution);

  static double evalPercentMapped(const Problem &problem, const Solution &solution);
  static double evalPercentDensity(const Problem &problem, const Solution &solution);

 private:
  SolutionChecker(const Problem &problem, const Solution &solution);
  void check();

  int nViolations();
  long long evalAreaViolation();
  long long evalAreaUsage();
  long long evalAreaMapped();
  long long evalTotalArea();

  int nItems();
  int nMappedItems();

  void checkPlate(const PlateSolution &plate);
  void checkCut(const CutSolution &cut);
  void checkRow(const RowSolution &row);
  void checkItem(const ItemSolution &row);

  void checkPlateDivision(const PlateSolution &plate);
  void checkCutDivision(const CutSolution &cut);
  void checkRowDivision(const RowSolution &row);

  void checkCutSize(const CutSolution &cut);
  void checkRowSize(const RowSolution &row);

  void checkItemUnicity();
  void checkSequences();

  bool fitsMinWaste(int a, int b) const;
  bool vCutIntersects(int x, const Defect &defect, int minY, int maxY) const;
  bool hCutIntersects(int y, const Defect &defect, int minX, int maxX) const;

  void reportErrors();
  void reportQuality();

  template<typename ... Args>
  void error(const std::string& type, const std::string& format, Args ... args );

 private:
  const Problem &problem_;
  const Solution &solution_;

  int plateId_;
  int cutId_;
  int rowId_;

  std::unordered_map<std::string, std::vector<std::string> > errors_;
};

#endif

