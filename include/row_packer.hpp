
#ifndef ROW_PACKER_HPP
#define ROW_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"

class RowPacker : Packer {
 public:
  static RowSolution run(const Packer &parent, Rectangle row, int start);
  static int count(const Packer &parent, Rectangle row, int start);

 private:
  RowPacker(const Packer &parent, Rectangle row, int start);
  RowSolution run();
  int count();
};

#endif

