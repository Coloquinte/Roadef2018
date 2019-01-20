// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

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
  RowPacker(const std::vector<Item> &sequence, SolverParams options);
  RowSolution run(Rectangle row, int start, const std::vector<Defect> &defects);
  RowDescription count(Rectangle row, int start, const std::vector<Defect> &defects);

 private:
  RowDescription countNoDefectsSimple();
  RowSolution runNoDefectsSimple();
  RowDescription countAsideDefectsSimple();
  RowSolution runAsideDefectsSimple();

  RowSolution runApproximate();
  RowSolution runExact();
  RowSolution runDiagnostic();
  RowDescription countApproximate();
  void reportFront(const std::vector<int> &front, const std::vector<int> &prev) const;

  void fillXData(RowDescription &description, int maxUsedX) const;
  void fillYData(RowDescription &description) const;
  bool fitsDimensionsAt(int minX, int width, int height) const;
  int earliestFit(int minX, int width, int height) const;
  bool canPlace(int x, int width, int height);
  bool canPlaceUp(int x, int width, int height);
  bool canPlaceDown(int x, int width, int height);
  bool isAdmissibleCutLine(int x) const;

  void checkSolution(const RowSolution &solution);
  void checkEquivalent(const RowDescription &description, const RowSolution &solution);

 private:
  std::unique_ptr<int[]> heights_;
};

#endif

