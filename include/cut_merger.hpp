// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef CUT_MERGER_HPP
#define CUT_MERGER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "row_merger.hpp"

class CutMerger : Merger {
 public:
  CutMerger(SolverParams options, const std::pair<std::vector<Item>, std::vector<Item> > &sequences);

  // Initialize with these starting points
  void init(Rectangle cut, const std::vector<Defect> &defects, std::pair<int, int> starts);
  void init(Rectangle cut, const std::vector<Defect> &defects, const std::vector<std::pair<int, int> > &starts);

  // Run the algorithm itself
  void buildFront();

  std::vector<std::pair<int, int> > getParetoFront() const;
  std::vector<std::pair<int, int> > optimisticParetoFront() const;
  CutSolution getSolution(std::pair<int, int> ends);

  void checkConsistency() const;
  long long nRowCalls() const { return nRowCalls_; }
  long long nPrunedRowCalls() const { return nPrunedRowCalls_; }

 private:
  void buildFrontExact();
  void buildFrontApproximate();
  void propagateFromElement(int ind);
  void propagateFrontToEnd();
  void checkSolution(const CutSolution &solution) const;

  bool isAdmissibleCutLine(int y) const;
  void makeAdmissible(int &y) const;
  int getMaxUsedY(const RowSolution &row) const;

  void runRowMerger(int minY, int maxY, std::pair<int, int> starts);
  void addMaxYCandidates(std::vector<int> &candidates, int minY, const std::vector<Item> &sequence, int start);
  std::vector<int> getMaxYCandidates(int minY, std::pair<int, int> starts);

  bool isEndDominated(int coord, std::pair<int, int> n) const;
  bool isRowDominated(int minY, int maxY, std::pair<int, int> starts);

  std::vector<int> getUsableItemAreas(const std::vector<Item> &sequence, int start) const;

 private:
  RowMerger rowMerger_;
  long long nRowCalls_;
  long long nPrunedRowCalls_;
};

#endif

