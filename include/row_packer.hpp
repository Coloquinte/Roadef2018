
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
  static RowSolution run(const Packer &parent, Rectangle row, int start);
  static Quality count(const Packer &parent, Rectangle row, int start);

 private:
  RowPacker(const Packer &parent, Rectangle row, int start);
  RowSolution run();
  Quality count();

  bool fitsDimensions(int width, int height) const;
  bool fitsDimensionsAt(int minX, int width, int height) const;

  bool fitsDefects(int width, int height) const;
  bool fitsDefectsAt(int minX, int width, int height) const;

  int earliestFit(int minX, int width, int height) const;

 private:
  int currentX_;
};

#endif

