
#ifndef CUT_PACKER_HPP
#define CUT_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"
#include "row_packer.hpp"
#include "pareto_front.hpp"

class CutPacker : Packer {
 public:
  static CutSolution run(const Packer &parent, Rectangle cut, int start);
  static int count(const Packer &parent, Rectangle cut, int start);

 private:
  CutPacker(const Packer &parent, Rectangle cut, int start);
  CutSolution run();
  int count();

  void propagate(int previousFront, int previousItems, int beginCoord);
  CutSolution backtrack();

  RowPacker::Quality countRow(int start, int minY, int maxY) const;
  RowSolution packRow(int start, int minY, int maxY) const;

  std::vector<int> computeBreakpoints() const;

 private:
  ParetoFront front_;
};

#endif

