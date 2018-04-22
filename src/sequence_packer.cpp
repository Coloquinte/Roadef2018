
#include "sequence_packer.hpp"
#include "utils.hpp"
#include "row_packer.hpp"

#include <cassert>
#include <algorithm>

/*
 * Dynamic programming on all cutting points
 *
 *
 * TODO:
 *  * prune infeasible cases
 *  * skip redundant cases (for example from solutions with whitespace)
 */

using namespace std;

Solution SequencePacker::run(const Problem &problem, const std::vector<Item> &sequence, int pitchX, int pitchY) {
  SequencePacker packer(problem, sequence, pitchX, pitchY);
  packer.run();
  return packer.solution_;
}

SequencePacker::SequencePacker(const Problem &problem, const std::vector<Item> &sequence, int pitchX, int pitchY)
: problem_(problem)
, sequence_(sequence)
, pitchX_(pitchX)
, pitchY_(pitchY) {
  packedItems_ = 0;
}

void SequencePacker::run() {
  Rectangle plateRect = Rectangle::FromCoordinates(0, 0, problem_.params().widthPlates, problem_.params().heightPlates);

  for (int i = 0; i < problem_.params().nPlates; ++i) {
    if (packedItems_ == (int) sequence_.size()) break;
    PlateSolution plate = packPlate(packedItems_, plateRect);
    packedItems_ += plate.nItems();
    solution_.plates.push_back(plate);
  }
}

PlateSolution SequencePacker::packPlate(int fromItem, Rectangle plate) {
  // Dynamic programming on the first-level cuts
  assert (plate.minX() == 0);
  assert (plate.minY() == 0);

  int maxCoord = plate.maxX();
  int maxIndex = divRoundUp(maxCoord, pitchX_);
  int minSpacing = divRoundUp(problem_.params().minXX, pitchX_);
  int maxSpacing = divRoundDown(problem_.params().maxXX, pitchX_);

  std::vector<int> packingVec(maxIndex + 1, fromItem);
  std::vector<int> previousVec(maxIndex + 1, 0);

  for (int end = minSpacing; end <= maxIndex; ++end) {
    int bestPacking = fromItem;
    int bestPrevious = max(0, end - maxSpacing);

    for (int begin = max(0, end - maxSpacing); begin <= end - minSpacing; ++begin) {
      int beginCoord = begin * pitchX_;
      int endCoord = min(end * pitchX_, maxCoord);
      Rectangle cut = Rectangle::FromCoordinates(beginCoord, 0, endCoord, plate.maxY());
      int previousItems = packingVec[begin];
      int cutCount = countPackCut(previousItems, cut);
      int packing = previousItems + cutCount;

      if (packing > bestPacking) {
        bestPacking = packing;
        bestPrevious = begin;
      }
    }

    packingVec[end] = bestPacking;
    previousVec[end] = bestPrevious;
  }

  // Backtrack for the best solution
  PlateSolution plateSolution(plate);
  int cur = maxIndex;
  // For the last plate, cut as soon as possible
  while (cur != 0 && packingVec[cur-1] == (int) sequence_.size())
    --cur;
  while (cur != 0) {
    int end = cur;
    int begin = previousVec[end];
    int beginCoord = begin * pitchX_;
    int endCoord = min(end * pitchX_, maxCoord);

    Rectangle cut = Rectangle::FromCoordinates(beginCoord, 0, endCoord, plate.maxY());
    auto solution = packCut(packingVec[begin], cut);
    assert (packingVec[begin] + solution.nItems() == packingVec[end]);
    plateSolution.cuts.push_back(solution);
    cur = begin;
  }
  reverse(plateSolution.cuts.begin(), plateSolution.cuts.end());

  return plateSolution;
}

int SequencePacker::countPackCut(int fromItem, Rectangle cut) {
  auto solution = packCut(fromItem, cut);
  return solution.nItems();
}

CutSolution SequencePacker::packCut(int fromItem, Rectangle cut) {
  // Dynamic programming on the rows i.e. second-level cuts
  assert (cut.minY() == 0);

  int maxCoord = cut.maxY();
  int maxIndex = divRoundUp(maxCoord, pitchY_);
  int minSpacing = divRoundUp(problem_.params().minYY, pitchY_);

  std::vector<int> packingVec(maxIndex + 1, fromItem);
  std::vector<int> previousVec(maxIndex + 1, 0);

  for (int end = minSpacing; end <= maxIndex; ++end) {
    int bestPacking = fromItem;
    int bestPrevious = 0;

    for (int begin = 0; begin <= end - minSpacing; ++begin) {
      int beginCoord = begin * pitchY_;
      int endCoord = min(end * pitchY_, maxCoord);
      Rectangle row = Rectangle::FromCoordinates(cut.minX(), beginCoord, cut.maxX(), endCoord);
      int packing = RowPacker::count(row, sequence_, packingVec[begin], problem_.params().minWaste).nItems;
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
  CutSolution cutSolution(cut);
  int cur = maxIndex;
  while (cur != 0) {
    int end = cur;
    int begin = previousVec[end];
    int beginCoord = begin * pitchY_;
    int endCoord = min(end * pitchY_, maxCoord);

    Rectangle row = Rectangle::FromCoordinates(cut.minX(), beginCoord, cut.maxX(), endCoord);
    auto solution = RowPacker::run(row, sequence_, packingVec[begin], problem_.params().minWaste);
    assert (packingVec[begin] + solution.nItems() == packingVec[end]);
    cutSolution.rows.push_back(solution);
    cur = begin;
  }
  reverse(cutSolution.rows.begin(), cutSolution.rows.end());

  return cutSolution;
}

bool SequencePacker::fitsMinWaste(int a, int b) const {
  return a == b
  || a <= b - problem_.params().minWaste;
}
