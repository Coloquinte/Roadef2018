
#ifndef ROW_PACKER_HPP
#define ROW_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "packer.hpp"

#include <memory>

class RowPacker : Packer {
 public:
  struct RowDescription : PlacementDescription {
    int nItems;

    RowDescription() {
      nItems = 0;
    }
  };

 public:
  RowPacker(const Problem &problem, const std::vector<Item> &sequence);

  RowSolution run(Rectangle row, int start, const std::vector<Defect> &defects);
  RowDescription count(Rectangle row, int start, const std::vector<Defect> &defects);

 private:
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

  void checkConsistency() const;
  void checkItems() const;
  void checkDefects() const;
  void checkSolution(const RowSolution &solution);
  void checkEquivalent(const RowDescription &description, const RowSolution &solution);

 private:
  std::unique_ptr<int[]> widths_;
  std::unique_ptr<int[]> heights_;
  std::unique_ptr<int[]> placements_;
};

#endif

