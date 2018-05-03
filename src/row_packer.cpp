
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
  currentX_ = region_.minX();
}

RowSolution RowPacker::run() {
  // Greedy placement
  // Attempt to place the item with the least possible usage
  // Not actually 100% correct due to the minWaste parameter at the end
  // The optimal solution involves the orientation of all items
  // And we'd need dynamic programming or brute-force for that
  RowSolution solution(region_);

  for (int i = start_; i < nItems(); ++i) {
    Item item = sequence_[i];
    int height = item.height;
    int width = item.width;
    int placement = currentX_;

    // TODO: push a bit further if it doesn't fit with the minimum waste
    int straightX = earliestFit(currentX_, width, height);
    bool fitsStraight = fitsDimensionsAt(straightX, width, height);

    int rotatedX  = earliestFit(currentX_, height, width);
    bool fitsRotated = fitsDimensionsAt(rotatedX, height, width);

    if (!fitsStraight && !fitsRotated)
      break;

    bool straightBetter = straightX + width <= rotatedX + height;
    if (fitsStraight && (!fitsRotated || straightBetter)) {
      placement = straightX;
    }
    else {
      placement = rotatedX;
      swap(width, height);
    }

    ItemSolution sol(placement, region_.minY(), width, height);
    sol.itemId = item.id;

    solution.items.push_back(sol);
    currentX_ = placement + width;
  }

  return solution;
}

RowPacker::Quality RowPacker::count() {
  int maxUsedY = region_.minY();

  int i = start_;
  for (; i < nItems(); ++i) {
    Item item = sequence_[i];
    int height = item.height;
    int width = item.width;
    int placement = currentX_;

    // TODO: push a bit further if it doesn't fit with the minimum waste
    int straightX = earliestFit(currentX_, width, height);
    bool fitsStraight = fitsDimensionsAt(straightX, width, height);

    int rotatedX  = earliestFit(currentX_, height, width);
    bool fitsRotated = fitsDimensionsAt(rotatedX, height, width);

    if (!fitsStraight && !fitsRotated)
      break;

    bool straightBetter = straightX + width <= rotatedX + height;
    if (fitsStraight && (!fitsRotated || straightBetter)) {
      placement = straightX;
    }
    else {
      placement = rotatedX;
      swap(width, height);
    }

    maxUsedY = max(maxUsedY, region_.minY() + height);
    currentX_ = placement + width;
  }

  return Quality {
    i - start_,
    maxUsedY
  };
}

bool RowPacker::fitsDimensions(int width, int height) const {
  return fitsDimensionsAt(currentX_, width, height);
}

bool RowPacker::fitsDefects(int width, int height) const {
  return fitsDefectsAt(currentX_, width, height);
}

bool RowPacker::fitsDimensionsAt(int minX, int width, int height) const {
  if (!fitsMinWaste(height, region_.height()))
    return false;
  if (!fitsMinWaste(minX + width, region_.maxX()))
    return false;
  return true;
}

bool RowPacker::fitsDefectsAt(int minX, int width, int height) const {
  Rectangle place = Rectangle::FromDimensions(minX, region_.minY(), width, height);
  for (Defect d : defects_) {
    if (place.intersects(d)) {
      return false;
    }
  }
  return true;
}

int RowPacker::earliestFit(int minX, int width, int height) const {
  int cur = minX;
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
    cur = max(minX + minWaste_, cur);
  }
}

