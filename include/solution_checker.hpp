// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef SOLUTION_CHECKER_HPP
#define SOLUTION_CHECKER_HPP

#include "solution.hpp"
#include "problem.hpp"

#include <string>
#include <memory>
#include <unordered_map>

class SolutionChecker {
 public:
  // Whole solution checking and reporting
  static int nViolations(const Problem &problem, const Solution &solution);
  static void report(const Problem &problem);
  static void report(const Problem &problem, const Solution &solution);

  // Statistics for solution quality evaluation
  static double evalPercentMapped(const Problem &problem, const Solution &solution);
  static double evalPercentDensity(const Problem &problem, const Solution &solution);

 private:
  SolutionChecker(const Problem &problem);
  const Problem& problem() const { return problem_; }

  int nViolations();
  long long evalAreaUsage(const Solution &solution);
  long long evalAreaMapped(const Solution &solution);
  long long evalTotalArea();
  long long evalPlateArea();

  int nItems();
  int nMappedItems(const Solution &solution);

  void checkSolution(const Solution &solution);
  void checkPlate(const PlateSolution &plate, bool lastPlate = false);
  void checkCut(const CutSolution &cut);
  void checkRow(const RowSolution &row);
  void checkItem(const ItemSolution &row);

  void checkPlateDivision(const PlateSolution &plate, bool lastPlate);
  void checkCutDivision(const CutSolution &cut);
  void checkRowDivision(const RowSolution &row);

  void checkCutSize(const CutSolution &cut);
  void checkRowSize(const RowSolution &row);

  void checkItemUnicity(const Solution &solution);
  void checkSequences(const Solution &solution);

  bool vCutIntersects(int x, const Defect &defect, int minY, int maxY) const;
  bool hCutIntersects(int y, const Defect &defect, int minX, int maxX) const;

  void reportErrors();
  void reportProblem();
  void reportQuality(const Solution &solution);

  template<typename ... Args>
  void error(const std::string& type, const std::string& format, Args ... args );

 private:
  const Problem &problem_;

  int plateId_;
  int cutId_;
  int rowId_;

  std::unordered_map<std::string, std::vector<std::string> > errors_;
};

#endif

