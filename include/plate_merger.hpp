
#ifndef PLATE_MERGER_HPP
#define PLATE_MERGER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "cut_merger.hpp"

class PlateMerger : Merger {
 public:
  PlateMerger(SolverParams options, const std::pair<std::vector<Item>, std::vector<Item> > &sequences);

  // Initialize with these starting points
  void init(Rectangle plate, const std::vector<Defect> &defects, std::pair<int, int> starts);
  void init(Rectangle plate, const std::vector<Defect> &defects, const std::vector<std::pair<int, int> > &starts);

  // Run the algorithm itself
  void buildFront();

  std::vector<std::pair<int, int> > getParetoFront(bool useAll=true) const;
  PlateSolution getSolution(std::pair<int, int> ends, bool useAll=true);
  std::pair<int, int> getStarts(std::pair<int, int> ends, bool useAll=true) const;

  void checkConsistency() const;
  long long nCalls() const { return nCalls_; }
  long long nCutCalls() const { return cutMerger_.nCalls(); }
  long long nRowCalls() const { return cutMerger_.nRowCalls(); }

 private:
  void buildFrontExact();
  void buildFrontApproximate();
  void propagateFromElement(int ind);
  void propagateFrontToEnd();
  void checkSolution(const PlateSolution &solution) const;
  int getEndFrontPos(std::pair<int, int> ends, bool useAll) const;

  bool isAdmissibleCutLine(int x) const;
  int findCuttingPosition(int from, int to) const;
  void makeAdmissible(int &x) const;
  int getMaxUsedX(const CutSolution &cut) const;

  void runCutMerger(int minX, int maxX, std::pair<int, int> starts);
  void addMaxXCandidates(std::vector<int> &candidates, int minX, const std::vector<Item> &sequence, int start);
  std::vector<int> getMaxXCandidates(int minX, std::pair<int, int> starts);

  bool isEndDominated(int coord, std::pair<int, int> n) const;
  bool isCutDominated(int minX, int maxX, std::pair<int, int> starts);

 private:
  CutMerger cutMerger_;
  long long nCalls_;
};

#endif

