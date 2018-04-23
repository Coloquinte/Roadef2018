
#ifndef CUT_PACKER_HPP
#define CUT_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"

class CutPacker : Packer {
 public:
  struct Quality {
    int nItems;
    int maxX;
  };

 public:
  // + pass defects
  static CutSolution run(const Packer &parent, Rectangle cut, int start);
  static Quality count(const Packer &parent, Rectangle cut, int start);

 private:
  CutPacker(const Packer &parent, Rectangle cut, int start);
  CutSolution run();
  Quality count();
};

#endif

