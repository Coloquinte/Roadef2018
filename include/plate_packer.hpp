
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

 private:
  PlatePacker(const Problem &problem, int plateId, const std::vector<Item> &sequence, int start);
  PlateSolution run();
  int count();

  int countCut(int start, int minX, int maxX) const;
  CutSolution packCut(int start, int minX, int maxX) const;
};

#endif

