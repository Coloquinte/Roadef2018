
#ifndef ROW_PACKER_HPP
#define ROW_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"

class RowPacker : Packer {
 public:
  struct Quality {
    int nItems;
    int maxUsedY;
  };

 public:
  RowPacker(const Problem &problem, const std::vector<Item> &sequence);
  RowSolution run(Rectangle row, int start, const std::vector<Defect> &defects);
  Quality count(Rectangle row, int start, const std::vector<Defect> &defects);

 private:
  bool fitsDimensions(int width, int height) const;
  bool fitsDimensionsAt(int minX, int width, int height) const;

  bool fitsDefects(int width, int height) const;
  bool fitsDefectsAt(int minX, int width, int height) const;

  int earliestFit(int minX, int width, int height) const;

 private:
  int currentX_;
};

#endif

