
#ifndef ROW_PACKER_HPP
#define ROW_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"

#include <memory>

class RowPacker : Packer {
 public:
  struct RowDescription {
    int nItems;
    int maxUsedX;
    int maxUsedY;
    bool tightX;
    bool tightY;

    RowDescription() {
      nItems = 0;
      maxUsedX = 0;
      maxUsedY = 0;
      tightX = true;
      tightY = true;
    }
  };

 public:
  RowPacker(const Problem &problem, const std::vector<Item> &sequence);
  RowSolution run(Rectangle row, int start, const std::vector<Defect> &defects);
  RowDescription count(Rectangle row, int start, const std::vector<Defect> &defects);

 private:
  void fillXData(RowDescription &description, int maxUsedX) const;
  void fillYData(RowDescription &description) const;
  bool fitsDimensionsAt(int minX, int width, int height) const;
  int earliestFit(int minX, int width, int height) const;

  RowDescription fitNoDefectsSimple();
  RowSolution solNoDefectsSimple();
  RowDescription fitNoDefectsExact();
  RowSolution solNoDefectsExact();
  RowDescription fitAsideDefectsSimple();
  RowSolution solAsideDefectsSimple();
  RowDescription fitAsideDefectsExact();
  RowSolution solAsideDefectsExact();

  void checkSolution(const RowSolution &solution);
  void checkEquivalent(const RowDescription &description, const RowSolution &solution);

 private:
  std::unique_ptr<int[]> widths_;
  std::unique_ptr<int[]> heights_;
  std::unique_ptr<int[]> placements_;
};

#endif

