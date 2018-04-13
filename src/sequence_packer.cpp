
#include "sequence_packer.hpp"

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

Solution SequencePacker::run(const Problem &problem, const std::vector<Item> &sequence) {
  SequencePacker packer(problem, sequence);
  packer.run();
  return packer.solution_;
}

SequencePacker::SequencePacker(const Problem &problem, const std::vector<Item> &sequence)
: problem_(problem)
, sequence_(sequence) {
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

  int maxX = plate.maxX();
  int minSpacing = problem_.params().minXX;
  int maxSpacing = problem_.params().maxXX;

  std::vector<int> packingVec(maxX + 1, fromItem);
  std::vector<int> previousXVec(maxX + 1, 0);

  for (int x_end = minSpacing; x_end <= maxX; ++x_end) {
    int bestPacking = fromItem;
    int bestPreviousX = max(0, x_end - maxSpacing);

    for (int x_begin = max(0, x_end - maxSpacing); x_begin <= x_end - minSpacing; ++x_begin) {
      Rectangle cut = Rectangle::FromCoordinates(x_begin, 0, x_end, plate.maxY());
      int cutCount = countPackCut(packingVec[x_begin], cut);
      if (cutCount == 0) break;
      int packing = packingVec[x_begin] + cutCount;

      if (packing > bestPacking) {
        bestPacking = packing;
        bestPreviousX = x_begin;
      }
    }

    packingVec[x_end] = bestPacking;
    previousXVec[x_end] = bestPreviousX;
  }

  // Backtrack for the best solution
  PlateSolution plateSolution(plate);
  int x = maxX;
  while (x != 0) {
    int x_end = x;
    int x_begin = previousXVec[x];

    Rectangle cut = Rectangle::FromCoordinates(x_begin, 0, x_end, plate.maxY());
    auto solution = packCut(packingVec[x_begin], cut);
    plateSolution.cuts.push_back(solution);
    x = x_begin;
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

  int maxY = cut.maxY();
  int minSpacing = problem_.params().minYY;

  std::vector<int> packingVec(maxY + 1, fromItem);
  std::vector<int> previousYVec(maxY + 1, 0);

  // TODO: early exit for the last plate
  for (int y_end = minSpacing; y_end <= maxY; ++y_end) {
    int bestPacking = fromItem;
    int bestPreviousY = 0;

    for (int y_begin = 0; y_begin <= y_end - minSpacing; ++y_begin) {
      Rectangle row = Rectangle::FromCoordinates(cut.minX(), y_begin, cut.maxX(), y_end);
      auto rowCount = countPackRow(packingVec[y_begin], row);
      if (rowCount == 0) break;
      int packing = packingVec[y_begin] + rowCount;

      if (packing > bestPacking) {
        bestPacking = packing;
        bestPreviousY = y_begin;
      }
    }

    packingVec[y_end] = bestPacking;
    previousYVec[y_end] = bestPreviousY;
  }

  // Backtrack for the best solution
  CutSolution cutSolution(cut);
  int y = maxY;
  while (y != 0) {
    int y_end = y;
    int y_begin = previousYVec[y];

    Rectangle row = Rectangle::FromCoordinates(cut.minX(), y_begin, cut.maxX(), y_end);

    auto solution = packRow(packingVec[y_begin], row);
    cutSolution.rows.push_back(solution);
    y = y_begin;
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
    int newX = currentX + width;
    if (!fitsMinWaste(newX, maxX))
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
