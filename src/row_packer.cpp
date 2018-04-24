
#include "row_packer.hpp"

using namespace std;

RowSolution RowPacker::run(const Packer &parent, Rectangle row, int start) {
  RowPacker packer(parent, row, start);
  return packer.run();
}

int RowPacker::count(const Packer &parent, Rectangle row, int start) {
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

int RowPacker::count() {
  int currentX = region_.minX();
  int rowHeight = region_.height();
  int maxX = region_.maxX();

  int i = start_;
  for (; i < nItems(); ++i) {
    //   Attempt to place the item with the best possible size
    //   Not actually 100% correct: due to the minimum waste
    // constraint at the end, the optimal solution involves
    // the orientation of all items. In practical cases,
    // it doesn't matter
    Item item = sequence_[i];
    int height = max(item.width, item.height);
    int width  = min(item.width, item.height);

    if (fitsMinWaste(height, rowHeight)) {
      if (fitsMinWaste(currentX + width, maxX))
        currentX += width;
      else
        break;
    }
    // Doesn't fit vertically; try rotating
    else if (fitsMinWaste(width, rowHeight)) {
      if (fitsMinWaste(currentX + height, maxX))
        currentX += height;
      else
        break;
    }
    else
      break;
  }

  return i - start_;
}

