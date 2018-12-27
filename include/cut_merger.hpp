
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
  CutSolution getSolution(std::pair<int, int> ends);

  void checkConsistency() const;

 private:
  void buildFrontExact();
  void buildFrontApproximate();
  void propagateFrontToEnd();
  void checkSolution(const CutSolution &solution) const;

  bool isAdmissibleCutLine(int y) const;

  void runRowMerger(int minY, int maxY, std::pair<int, int> starts);
  std::vector<int> getMaxYCandidates(int minY, std::pair<int, int> starts);

 private:
  RowMerger rowMerger_;
};

#endif

