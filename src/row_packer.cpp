
#include "row_packer.hpp"

#include <cassert>

using namespace std;

RowSolution RowPacker::run(const Packer &parent, Rectangle row, int start) {
  RowPacker packer(parent, row, start);
  return packer.run();
}

RowPacker::Quality RowPacker::count(const Packer &parent, Rectangle row, int start) {
  RowPacker packer(parent, row, start);
  return packer.count();
}

RowPacker::RowPacker(const Packer &parent, Rectangle row, int start)
: Packer(parent) {
  region_ = row;
  start_ = start;
}

RowSolution RowPacker::run() {
  // Greedy placement
  RowSolution solution(region_);

  int currentX = region_.minX();
  for (int i = start_; i < nItems(); ++i) {
    // Attempt to place the item with the best possible size
    Item item = sequence_[i];
    int height = max(item.width, item.height);
    int width = min(item.width, item.height);
    // Doesn't fit vertically; try rotating
    if (!fitsMinWaste(height, region_.height()))
      swap(width, height);
    // Still doesn't fit
    if (!fitsMinWaste(height, region_.height()))
      break;

    // Not actually 100% correct due to the minWaste parameter
    // The optimal solution involves the orientation of all items
    // And we'd need dynamic programming or brute-force for that
    int newX = currentX + width;
    if (!fitsMinWaste(newX, region_.maxX()))
      break;

    ItemSolution sol(currentX, region_.minY(), width, height);
    sol.itemId = item.id;

    solution.items.push_back(sol);
    currentX = newX;
  }

  return solution;
}

RowPacker::Quality RowPacker::count() {
  int currentX = region_.minX();
  int i = start_;
  int maxUsedY = region_.minY();
  for (; i < nItems(); ++i) {
    //   Attempt to place the item with the best possible size
    //   Not actually 100% correct: due to the minimum waste
    // constraint at the end, the optimal solution involves
    // the orientation of all items. In practical cases,
    // it doesn't matter
    Item item = sequence_[i];
    int height = max(item.width, item.height);
    int width  = min(item.width, item.height);

    // Doesn't fit vertically; try rotating
    if (!fitsMinWaste(height, region_.height()))
      swap(width, height);
    // Still doesn't fit
    if (!fitsMinWaste(height, region_.height()))
      break;

    // Not actually 100% correct due to the minWaste parameter
    // The optimal solution involves the orientation of all items
    // And we'd need dynamic programming or brute-force for that
    int newX = currentX + width;
    if (!fitsMinWaste(newX, region_.maxX()))
      break;

    maxUsedY = max(region_.minY() + height, maxUsedY);
    currentX = newX;
  }

  //assert (run().nItems() == i - start_);
  //assert (run().maxUsedY() == maxUsedY);

  return Quality {
    i - start_,
    maxUsedY
  };
}

