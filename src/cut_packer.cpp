
#include "cut_packer.hpp"
#include "row_packer.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

CutSolution CutPacker::run(const Packer &parent, Rectangle cut, int start) {
  CutPacker packer(parent, cut, start);
  return packer.run();
}

CutPacker::Quality CutPacker::count(const Packer &parent, Rectangle cut, int start) {
  CutPacker packer(parent, cut, start);
  return packer.count();
}

CutPacker::CutPacker(const Packer &parent, Rectangle cut, int start)
: Packer(parent) {
  region_ = cut;
  start_ = start;
}

CutSolution CutPacker::run() {
  // Dynamic programming on the rows i.e. second-level cuts
  assert (region_.minY() == 0);

  int maxCoord = region_.maxY();
  int maxIndex = divRoundUp(maxCoord, pitchY_);
  int minSpacing = divRoundUp(minYY_, pitchY_);

  std::vector<int> packingVec(maxIndex + 1, start_);
  std::vector<int> previousVec(maxIndex + 1, 0);

  for (int end = minSpacing; end <= maxIndex; ++end) {
    int bestPacking = start_;
    int bestPrevious = 0;

    for (int begin = 0; begin <= end - minSpacing; ++begin) {
      int beginCoord = begin * pitchY_;
      int endCoord = min(end * pitchY_, maxCoord);
      Rectangle row = Rectangle::FromCoordinates(region_.minX(), beginCoord, region_.maxX(), endCoord);
      int packing = RowPacker::count(row, sequence_, packingVec[begin], minWaste_).nItems;
      assert (packing >= packingVec[begin]);

      if (packing > bestPacking) {
        bestPacking = packing;
        bestPrevious = begin;
      }
    }

    packingVec[end] = bestPacking;
    previousVec[end] = bestPrevious;
  }

  // Backtrack for the best solution
  CutSolution cutSolution(region_);
  int cur = maxIndex;
  while (cur != 0) {
    int end = cur;
    int begin = previousVec[end];
    int beginCoord = begin * pitchY_;
    int endCoord = min(end * pitchY_, maxCoord);

    Rectangle row = Rectangle::FromCoordinates(region_.minX(), beginCoord, region_.maxX(), endCoord);
    auto solution = RowPacker::run(row, sequence_, packingVec[begin], minWaste_);
    assert (packingVec[begin] + solution.nItems() == packingVec[end]);
    cutSolution.rows.push_back(solution);
    cur = begin;
  }
  reverse(cutSolution.rows.begin(), cutSolution.rows.end());

  return cutSolution;
}

CutPacker::Quality CutPacker::count() {
  CutSolution solution = run();
  int nItems = solution.nItems();
  int maxX = region_.minX();
  for (const RowSolution row : solution.rows) {
    for (ItemSolution item : row.items) {
      maxX = max(maxX, item.maxX());
    }
  }
  return CutPacker::Quality { nItems, maxX };
}

