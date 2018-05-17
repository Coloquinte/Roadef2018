
#ifndef PLATE_PACKER_HPP
#define PLATE_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"
#include "cut_packer.hpp"

class PlatePacker : Packer {
 public:
  static PlateSolution run(const Problem &problem, int plateId, const std::vector<Item> &sequence, int start);
  static int count(const Problem &problem, int plateId, const std::vector<Item> &sequence, int start);

  PlatePacker(const Problem &problem, const std::vector<Item> &sequence);
  PlateSolution run(int plateId, int start);
  int count(int plateId, int start);

 private:
  void propagate(int previousFront, int previousItems, int beginCoord);
  void propagateBreakpoints(int after);
  PlateSolution backtrack();

  int countCut(int start, int minX, int maxX);
  CutSolution packCut(int start, int minX, int maxX);

 private:
  CutPacker cutPacker_;
  ParetoFront front_;
  std::vector<int> slices_;
  const Problem &problem_;
};

#endif

