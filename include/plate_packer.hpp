
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

  void setup(int plateId, int start);
  void propagate(int previousFront, int beginCoord);
  void propagateBreakpoints(int after);
  PlateSolution backtrack();

  CutPacker::CutDescription countCut(int start, int minX, int maxX);
  CutSolution packCut(int start, int minX, int maxX);

  bool isAdmissibleCutLine(int x) const;

 private:
  CutPacker cutPacker_;
  PackerFront front_;
  std::vector<int> slices_;
  const Problem &problem_;
};

#endif

