
#ifndef ROW_PACKER_NG_HPP
#define ROW_PACKER_NG_HPP

#include "problem.hpp"
#include "solution.hpp"

#include <memory>

struct PlacementDescription {
  int maxUsedX;
  int maxUsedY;
  // Whether a minWaste gap is needed
  bool tightX;
  bool tightY;

  PlacementDescription() {
    maxUsedX = 0;
    maxUsedY = 0;
    tightX = true;
    tightY = true;
  }
};

class RowPackerNg {
 public:
  struct RowDescription : PlacementDescription {
    int nItems;

    RowDescription() {
      nItems = 0;
    }
  };

 public:
  RowPackerNg(int maxSequenceSize);
  
  // Fit with no defects; linear but may miss some edge cases
  RowDescription fitNoDefectsSimple(Rectangle row, const Item *items, int nItems);
  RowSolution solNoDefectsSimple(Rectangle row, const Item *items, int nItems);

  // Fit with no defects; exact and potentially exponential
  RowDescription fitNoDefectsExact(Rectangle row, const Item *items, int nItems);
  RowSolution solNoDefectsExact(Rectangle row, const Item *items, int nItems);

  // Fit with defects, but not above one; linear but may miss some edge cases
  RowDescription fitAsideDefectsSimple(Rectangle row, const Item *items, int nItems, const Defect *defects, int nDefects);
  RowSolution solAsideDefectsSimple(Rectangle row, const Item *items, int nItems, const Defect *defects, int nDefects);

  // Fit with defects, but not above one; exact and potentially exponential
  RowDescription fitAsideDefectsExact(Rectangle row, const Item *items, int nItems, const Defect *defects, int nDefects);
  RowSolution solAsideDefectsExact(Rectangle row, const Item *items, int nItems, const Defect *defects, int nDefects);

  static void checkEquivalent(const RowDescription &description, const RowSolution &solution);

 private:
  RowDescription fitNoDefectsSimple();
  RowSolution solNoDefectsSimple();
  RowDescription fitNoDefectsExact();
  RowSolution solNoDefectsExact();
  RowDescription fitAsideDefectsSimple();
  RowSolution solAsideDefectsSimple();
  RowDescription fitAsideDefectsExact();
  RowSolution solAsideDefectsExact();

 private:
  void checkConsistency() const;
  void checkItems() const;
  void checkDefects() const;
  void checkSolution(const RowSolution &solution);

  void init(int maxSequenceSize);
  void reset(Rectangle row, const Item *items, int nItems);
  void reset(Rectangle row, const Item *items, int nItems, const Defect *defects, int nDefects);

  // Computation helper
  void fillYData(RowDescription &description) const;
  bool fitsDimensionsAt(int minX, int width, int height) const;
  int earliestFit(int minX, int width, int height) const;

 private:
  Rectangle row_;
  const Item *items_;
  int nItems_;
  const Defect *defects_;
  int nDefects_;

  // Work data
  int maxSequenceSize_;
  std::unique_ptr<int[]> widths_;
  std::unique_ptr<int[]> heights_;
  std::unique_ptr<int[]> placements_;
};

#endif


