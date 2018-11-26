
#include "row_packer.hpp"
#include "utils.hpp"

#include <cassert>

using namespace std;

RowPacker::RowPacker(const Problem &problem, const vector<Item> &sequence)
: Packer(problem, sequence) {
  widths_.reset(new int[sequence.size()]);
  heights_.reset(new int[sequence.size()]);
  placements_.reset(new int[sequence.size()]);
}

RowPacker::RowDescription RowPacker::fitNoDefectsSimple() {
  const int width = region_.width();
  const int height = region_.height();
  int left = width;
  RowDescription description;
  for (int i = start_; i < nItems(); ++i) {
    Item item = sequence_[i];;
    if (utils::fitsMinWaste(item.height, height)
     && utils::fitsMinWaste(item.width, left)) {
      left -= item.width;
      heights_[i] = item.height;
    }
    else if (utils::fitsMinWaste(item.width, height)
          && utils::fitsMinWaste(item.height, left)) {
      left -= item.height;
      heights_[i] = item.width;
    }
    else {
      break;
    }
    ++description.nItems;
  }
  description.maxUsedX = region_.maxX() - left;
  description.tightX = true;
  fillYData(description);
  return description;
}

RowSolution RowPacker::solNoDefectsSimple() {
  RowSolution solution(region_);
  int width = region_.width();
  int height = region_.height();
  int left = width;
  for (int i = start_; i < nItems(); ++i) {
    Item item = sequence_[i];;
    if (utils::fitsMinWaste(item.height, height)
     && utils::fitsMinWaste(item.width, left)) {
      Rectangle r = Rectangle::FromDimensions(region_.maxX() - left, region_.minY(), item.width, item.height);
      solution.items.emplace_back(r, item.id);
      left -= item.width;
    }
    else if (utils::fitsMinWaste(item.width, height)
          && utils::fitsMinWaste(item.height, left)) {
      Rectangle r = Rectangle::FromDimensions(region_.maxX() - left, region_.minY(), item.height, item.width);
      solution.items.emplace_back(r, item.id);
      left -= item.height;
    }
    else {
      break;
    }
  }

  checkSolution(solution);
  return solution;
}

RowPacker::RowDescription RowPacker::fitAsideDefectsSimple() {
  if (nDefects() == 0) return fitNoDefectsSimple();

  int currentX = region_.minX();
  RowDescription description;
  for (int i = start_; i < nItems(); ++i) {
    Item item = sequence_[i];;
    int height = item.height;
    int width = item.width;

    int placement = earliestFit(currentX, item.width, item.height);
    bool fits = fitsDimensionsAt(placement, item.width, item.height);

    if (!fits || placement + item.width > currentX + item.height) {
      int rotatedX  = earliestFit(currentX, item.height, item.width);
      bool fitsRotated = fitsDimensionsAt(rotatedX, item.height, item.width);

      bool rotatedBetter = placement + item.width > rotatedX + item.height;
      if (fitsRotated && (!fits || rotatedBetter)) {
        placement = rotatedX;
        swap(width, height);
        fits = true;
      }
    }

    if (!fits)
      break;

    heights_[i] = height;
    currentX = placement + width;
    ++description.nItems;
  }

  description.maxUsedX = currentX;
  description.tightX = true;
  fillYData(description);
  assert (description.maxUsedX >= region_.minX());
  assert (description.maxUsedY >= region_.minY());
  assert (description.maxUsedX <= region_.maxX());
  assert (description.maxUsedY <= region_.maxY());

  return description;
}

RowSolution RowPacker::solAsideDefectsSimple() {
  if (nDefects() == 0) return solNoDefectsSimple();

  int currentX = region_.minX();
  RowSolution solution(region_);
  for (int i = start_; i < nItems(); ++i) {
    Item item = sequence_[i];;
    int height = item.height;
    int width = item.width;

    int placement = earliestFit(currentX, item.width, item.height);
    bool fits = fitsDimensionsAt(placement, item.width, item.height);

    if (!fits || placement + item.width > currentX + item.height) {
      int rotatedX  = earliestFit(currentX, item.height, item.width);
      bool fitsRotated = fitsDimensionsAt(rotatedX, item.height, item.width);

      bool rotatedBetter = placement + item.width > rotatedX + item.height;
      if (fitsRotated && (!fits || rotatedBetter)) {
        placement = rotatedX;
        swap(width, height);
        fits = true;
      }
    }

    if (!fits)
      break;

    Rectangle r = Rectangle::FromDimensions(placement, region_.minY(), width, height);
    solution.items.emplace_back(r, item.id);
    currentX = placement + width;
  }

  return solution;
}

void RowPacker::fillYData(RowDescription &description) const {
  int maxHeight = 0;
  for (int i = start_; i < start_ + description.nItems; ++i) {
    maxHeight = max(heights_[i], maxHeight);
  }
  description.maxUsedY = region_.minY() + maxHeight;
  description.tightY = true;
  for (int i = start_; i < start_ + description.nItems; ++i) {
    if (heights_[i] == maxHeight)
      continue;
    if (heights_[i] + Params::minWaste > maxHeight)
      description.tightY = false;
  }
  if (!description.tightY) description.maxUsedY += Params::minWaste;
  int lowestCut = lowestHorizontalCut(description.maxUsedY, description.tightY);
  if (lowestCut != description.maxUsedY) {
    description.maxUsedY = lowestCut;
    description.tightY = false;
  }
}

bool RowPacker::fitsDimensionsAt(int minX, int width, int height) const {
  if (!utils::fitsMinWaste(height, region_.height()))
    return false;
  if (!utils::fitsMinWaste(minX + width, region_.maxX()))
    return false;
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
      if (d.intersectsVerticalLine(place.minX())) {
        cur = max(d.maxX() + 1, cur);
        hasDefect = true;
      }
      if (d.intersectsVerticalLine(place.maxX())) {
        cur = max(d.maxX() + 1 - width, cur);
        hasDefect = true;
      }
    }
    if (!hasDefect)
      return cur;
    cur = max(minX + Params::minWaste, cur);
  }
}

int RowPacker::lowestHorizontalCut(int minY, bool tightY) const {
  int cur = minY;
  while (true) {
    bool hasDefect = false;
    for (Defect d : defects_) {
      if (d.intersectsHorizontalLine(cur)) {
        cur = max(d.maxY() + 1, cur);
        hasDefect = true;
      }
    }
    if (!hasDefect)
      return cur;
    cur = max(tightY ? minY + Params::minWaste : minY, cur);
  }
}

void RowPacker::checkConsistency() const {
  checkItems();
  checkDefects();
}

void RowPacker::checkItems() const {
  for (int i = 0; i < nItems(); ++i) {
    assert (sequence_[i].height >= sequence_[i].width);
  }
}

void RowPacker::checkDefects() const {
  for (Defect defect : defects_) {
    assert (region_.contains(defect));
  }
}

void RowPacker::checkSolution(const RowSolution &row) {
  if (row.items.empty()) return;

  for (const ItemSolution &item : row.items) {
    assert (row.contains(item));
    assert (utils::fitsMinWaste(row.minY(), item.minY()));
    assert (utils::fitsMinWaste(item.maxY(), row.maxY()));
    assert (item.maxY() == row.maxY() || item.minY() == row.minY());
    for (Defect defect : defects_) {
      assert (!defect.intersectsVerticalLine(item.minX()));
      assert (!defect.intersectsVerticalLine(item.maxX()));
    }
  }
  for (Defect defect : defects_) {
    assert (!defect.intersectsHorizontalLine(row.minY()));
    assert (!defect.intersectsHorizontalLine(row.maxY()));
    assert (!defect.intersectsVerticalLine(row.minX()));
    assert (!defect.intersectsVerticalLine(row.maxX()));
  }

  assert (utils::fitsMinWaste(row.minX(), row.items.front().minX()));
  assert (utils::fitsMinWaste(row.items.back().maxX(), row.maxX()));

  for (int i = 0; i+1 < (int) row.items.size(); ++i) {
    ItemSolution item1 = row.items[i];
    ItemSolution item2 = row.items[i+1];
    assert (item1.maxX() >= item2.minX());
    assert (utils::fitsMinWaste(item1.maxX(), item2.minX()));
  }
}

void RowPacker::checkEquivalent(const RowDescription &description, const RowSolution &solution) {
  assert (description.nItems == (int) solution.items.size());
  // No other check here since the defects mess it up
}

RowSolution RowPacker::run(Rectangle row, int start, const std::vector<Defect> &defects) {
  init(row, start, defects);
  return solAsideDefectsSimple();
}

RowPacker::RowDescription RowPacker::count(Rectangle row, int start, const std::vector<Defect> &defects) {
  init(row, start, defects);
  return fitAsideDefectsSimple();
}

