
#include "sequence_packer.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

/*
 * Dynamic programming on all cutting points
 *
 *
 * TODO:
 *  * prune infeasible cases
 *  * skip redundant cases (for example from solutions with whitespace)
 *  * grid pitch reduction
 *  * counting-only versions (no dynamic allocation)
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
      int previousItems = packingVec[begin];
      int rowCount = countPackRow(previousItems, row);
      int packing = previousItems + rowCount;

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
    auto solution = packRow(packingVec[begin], row);
    assert (packingVec[begin] + solution.nItems() == packingVec[end]);
    cutSolution.rows.push_back(solution);
    cur = begin;
  }
  reverse(cutSolution.rows.begin(), cutSolution.rows.end());

  return cutSolution;
}

int SequencePacker::countPackRow(int fromItem, Rectangle row) {
  int cnt = 0;
  int currentX = row.minX();
  int rowHeight = row.height();
  int maxX = row.maxX();
  for (int i = fromItem; i < nItems(); ++i) {
    // Attempt to place the item with the best possible size
    Item item = sequence_[i];
    int height = max(item.width, item.height);
    int width = min(item.width, item.height);

    // Doesn't fit vertically; try rotating
    if (!fitsMinWaste(height, rowHeight))
      swap(width, height);
    // Still doesn't fit
    if (!fitsMinWaste(height, rowHeight))
      break;
    
    // Not actually 100% correct due to the minWaste parameter
    // The optimal solution involves the orientation of all items
    // And we'd need dynamic programming or brute-force for that
    currentX += width;
    if (!fitsMinWaste(currentX, maxX))
      break;
    ++cnt;
  }

  return cnt;
}

RowSolution SequencePacker::packRow(int fromItem, Rectangle row) {
  // Greedy placement
  RowSolution solution(row);

  int currentX = row.minX();
  for (int i = fromItem; i < nItems(); ++i) {
    // Attempt to place the item with the best possible size
    Item item = sequence_[i];
    int height = max(item.width, item.height);
    int width = min(item.width, item.height);
    // Doesn't fit vertically; try rotating
    if (!fitsMinWaste(height, row.height()))
      swap(width, height);
    // Still doesn't fit
    if (!fitsMinWaste(height, row.height()))
      break;

    // Not actually 100% correct due to the minWaste parameter
    // The optimal solution involves the orientation of all items
    // And we'd need dynamic programming or brute-force for that
    int newX = currentX + width;
    if (!fitsMinWaste(newX, row.maxX()))
      break;

    ItemSolution sol(currentX, row.minY(), width, height);
    sol.itemId = item.id;

    solution.items.push_back(sol);
    currentX = newX;
  }

  return solution;
}

bool SequencePacker::fitsMinWaste(int a, int b) const {
  return a == b
  || a <= b - problem_.params().minWaste;
}
