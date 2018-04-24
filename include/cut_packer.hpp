
#ifndef CUT_PACKER_HPP
#define CUT_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"

class CutPacker : Packer {
 public:
  static CutSolution run(const Packer &parent, Rectangle cut, int start);
  static int count(const Packer &parent, Rectangle cut, int start);

 private:
  CutPacker(const Packer &parent, Rectangle cut, int start);
  CutSolution run();
  int count();
};

#endif

