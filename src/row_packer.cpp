
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
    int height = item.height;
    int width = item.width;
    int placement = currentX;
    assert (width <= height);

    // Doesn't fit vertically; try rotating
    if (!fitsMinWaste(height, region_.height()))
      swap(width, height);
    // Still doesn't fit
    if (!fitsMinWaste(height, region_.height()))
      break;

    if (!fitsDefects(currentX, width, height)) {
      int straightX = earliestFit(currentX, width, height);
      int rotatedX  = earliestFit(currentX, height, width);
      bool fitsStraight = fitsMinWaste(height, region_.height());
      bool fitsRotated = fitsMinWaste(width, region_.height());
      bool useStraight = !fitsRotated || (fitsStraight && straightX <= rotatedX);
      if (useStraight) {
        placement = straightX;
      }
      else {
        assert (fitsRotated);
        placement = rotatedX;
        swap(width, height);
      }
    }

    int newX = placement + width;
    // Not actually 100% correct due to the minWaste parameter
    // The optimal solution involves the orientation of all items
    // And we'd need dynamic programming or brute-force for that
    if (!fitsMinWaste(newX, region_.maxX()))
      break;

    ItemSolution sol(placement, region_.minY(), width, height);
    sol.itemId = item.id;

    solution.items.push_back(sol);
    currentX = newX;
  }

  return solution;
}

RowPacker::Quality RowPacker::count() {
  RowSolution solution = run();
  return Quality {
    solution.nItems(),
    solution.maxUsedY()
  };
}

bool RowPacker::fitsDefects(int from, int width, int height) const {
  Rectangle place = Rectangle::FromDimensions(from, region_.minY(), width, height);
  for (Defect d : defects_) {
    if (place.intersects(d)) {
      return false;
    }
  }
  return true;
}

int RowPacker::earliestFit(int from, int width, int height) const {
  int cur = from;
  while (true) {
    Rectangle place = Rectangle::FromDimensions(cur, region_.minY(), width, height);
    bool hasDefect = false;
    for (Defect d : defects_) {
      if (place.intersects(d)) {
        cur = max(d.maxX() + 1, cur);
        hasDefect = true;
      }
    }
    if (!hasDefect)
      return cur;
    cur = max(from + minWaste_, cur);
  }
}

