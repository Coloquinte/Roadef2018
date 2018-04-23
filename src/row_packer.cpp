
#include "row_packer.hpp"

using namespace std;

RowSolution RowPacker::run(Rectangle row, const std::vector<Item> &sequence, int start, int minWaste) {
  RowPacker packer(row, sequence, start, minWaste);
  return packer.run();
}

RowPacker::Quality RowPacker::count(Rectangle row, const std::vector<Item> &sequence, int start, int minWaste) {
  RowPacker packer(row, sequence, start, minWaste);
  return packer.count();
}

RowPacker::RowPacker(Rectangle row, const std::vector<Item> &sequence, int start, int minWaste)
: row_(row)
, sequence_(sequence)
, start_(start)
, minWaste_(minWaste) {
}

RowSolution RowPacker::run() {
  // Greedy placement
  RowSolution solution(row_);

  int currentX = row_.minX();
  for (int i = start_; i < nItems(); ++i) {
    // Attempt to place the item with the best possible size
    Item item = sequence_[i];
    int height = max(item.width, item.height);
    int width = min(item.width, item.height);
    // Doesn't fit vertically; try rotating
    if (!fitsMinWaste(height, row_.height()))
      swap(width, height);
    // Still doesn't fit
    if (!fitsMinWaste(height, row_.height()))
      break;

    // Not actually 100% correct due to the minWaste parameter
    // The optimal solution involves the orientation of all items
    // And we'd need dynamic programming or brute-force for that
    int newX = currentX + width;
    if (!fitsMinWaste(newX, row_.maxX()))
      break;

    ItemSolution sol(currentX, row_.minY(), width, height);
    sol.itemId = item.id;

    solution.items.push_back(sol);
    currentX = newX;
  }

  return solution;
}

RowPacker::Quality RowPacker::count() {
  int currentX = row_.minX();
  int rowHeight = row_.height();
  int maxX = row_.maxX();

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

  return Quality { i, currentX };
}

