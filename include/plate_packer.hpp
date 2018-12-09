
#ifndef PLATE_PACKER_HPP
#define PLATE_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"
#include "packer_front.hpp"
#include "cut_packer.hpp"

class PlatePacker : Packer {
 public:
  static PlateSolution run(const Problem &problem, const std::vector<Item> &sequence, SolverParams options, int plateId, int start);

  PlatePacker(const Problem &problem, const std::vector<Item> &sequence, SolverParams options);
  PlateSolution run(int plateId, int start);

 private:
  PlateSolution runApproximate();
  PlateSolution runExact();
  PlateSolution runDiagnostic();
  void reportFront(const std::vector<int> &front, const std::vector<int> &prev) const;

  void setup(int plateId, int start);
  void propagate(int previousFront, int beginCoord);
  void propagateBreakpoints(int after);
  PlateSolution backtrack();

  CutPacker::CutDescription countCut(int start, int minX, int maxX);
  CutSolution packCut(int start, int minX, int maxX);

  bool isAdmissibleCutLine(int x) const;
  int findCuttingPositionTowards(int endPos) const;
  void insertInFront(int begin, int end, int totalItems, int previous);

 private:
  CutPacker cutPacker_;
  PackerFront front_;
  std::vector<int> slices_;
  const Problem &problem_;
};

#endif

