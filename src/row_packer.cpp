
#include "row_packer.hpp"

#include <cassert>

using namespace std;

RowPacker::RowPacker(const Problem &problem, const vector<Item> &sequence)
: Packer(problem, sequence) {
  currentX_ = 0;
}

RowSolution RowPacker::run(Rectangle row, int start, const std::vector<Defect> &defects) {
  init(row, start, defects);
  currentX_ = region_.minX();
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

    int placement = earliestFit(currentX_, width, height);
    bool fits = fitsDimensionsAt(placement, width, height);

    if (!fits || placement + width > currentX_ + height) {
      int rotatedX  = earliestFit(currentX_, height, width);
      bool fitsRotated = fitsDimensionsAt(rotatedX, height, width);

      bool rotatedBetter = placement + width > rotatedX + height;
      if (fitsRotated && (!fits || rotatedBetter)) {
        placement = rotatedX;
        swap(width, height);
        fits = true;
      }
    }

    if (!fits)
      break;

    ItemSolution sol(placement, region_.minY(), width, height);
    sol.itemId = item.id;

    solution.items.push_back(sol);
    currentX_ = placement + width;
  }

  return solution;
}

RowPacker::Quality RowPacker::count(Rectangle row, int start, const std::vector<Defect> &defects) {
  init(row, start, defects);
  currentX_ = region_.minX();
  int maxUsedY = region_.minY();

  int i = start_;
  for (; i < nItems(); ++i) {
    Item item = sequence_[i];
    int height = item.height;
    int width = item.width;

    int placement = earliestFit(currentX_, width, height);
    bool fits = fitsDimensionsAt(placement, width, height);

    if (!fits || placement + width > currentX_ + height) {
      int rotatedX  = earliestFit(currentX_, height, width);
      bool fitsRotated = fitsDimensionsAt(rotatedX, height, width);

      bool rotatedBetter = placement + width > rotatedX + height;
      if (fitsRotated && (!fits || rotatedBetter)) {
        placement = rotatedX;
        swap(width, height);
        fits = true;
      }
    }

    if (!fits)
      break;

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
  // TODO: optimize this part
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

