// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#include "row_packer.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

using namespace std;

RowPacker::RowPacker(const vector<Item> &sequence, SolverParams options)
: Packer(sequence, options) {
  heights_.reset(new int[sequence.size()]);
}

RowSolution RowPacker::run(Rectangle row, int start, const vector<Defect> &defects) {
  init(row, start, defects);
  checkConsistency();
  if (options_.rowPacking == PackingOption::Approximate) {
    return runApproximate();
  }
  else if (options_.rowPacking == PackingOption::Exact) {
    return runExact();
  }
  else {
    return runDiagnostic();
  }
}

RowPacker::RowDescription RowPacker::count(Rectangle row, int start, const vector<Defect> &defects) {
  init(row, start, defects);
  checkConsistency();
  if (options_.rowPacking != PackingOption::Approximate
   && options_.cutPacking != PackingOption::Approximate
   && options_.platePacking != PackingOption::Approximate) {
    // Since maxUsedX/maxUsedY are not properly computed, only allowed whenever everything is exact
    RowSolution solution = runExact();
    RowDescription desc;
    desc.nItems = solution.nItems();
    desc.maxUsedX = region_.maxX();
    desc.maxUsedY = region_.maxY();
    return desc;
  }
  else {
    return countApproximate();
  }
}

RowPacker::RowDescription RowPacker::countNoDefectsSimple() {
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
  fillXData(description, region_.maxX() - left);
  fillYData(description);
  return description;
}

RowSolution RowPacker::runNoDefectsSimple() {
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

RowPacker::RowDescription RowPacker::countAsideDefectsSimple() {
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

  fillXData(description, currentX);
  fillYData(description);

  assert (description.maxUsedX >= region_.minX() + Params::minXX);
  assert (description.maxUsedY >= region_.minY() + Params::minYY);
  assert (description.maxUsedX <= region_.maxX());
  assert (description.maxUsedY <= region_.maxY());
  assert (!description.tightY || utils::fitsMinWaste(description.maxUsedY, region_.maxY()));

  return description;
}

RowSolution RowPacker::runAsideDefectsSimple() {
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

RowPacker::RowDescription RowPacker::countApproximate() {
  if (nDefects() == 0) return countNoDefectsSimple();
  else return countAsideDefectsSimple();
}

RowSolution RowPacker::runApproximate() {
  if (nDefects() == 0) return runNoDefectsSimple();
  else return runAsideDefectsSimple();
}

RowSolution RowPacker::runExact() {
  vector<int> front(region_.maxX() + 1, start_);
  vector<int> prev(region_.maxX() + 1, 0);
  // Fill the front
  front[region_.minX()] = start_;
  vector<int> firstPresent;
  for (int i = region_.minX(); i <= region_.maxX(); ++i) {
    if (!isAdmissibleCutLine(i)) continue;
    // Take an empty cut before into account
    for (int b = firstPresent.size(); b > 0; --b) {
      if (start_ + b > front[i] && firstPresent[b-1] + Params::minWaste <= i) {
        front[i] = start_ + b;
        prev[i] = firstPresent[b-1];
      }
    }
    int cnt = front[i];
    // First appearance of this item
    if (cnt - start_ > (int) firstPresent.size())
      firstPresent.push_back(i);
    // Now extend the front
    if (cnt == nItems()) continue;
    Item item = sequence_[cnt];
    // Try to place not rotated
    if (i + item.width <= region_.maxX()
     && front[i + item.width] <= cnt
     && utils::fitsMinWaste(item.height, region_.height())
     && isAdmissibleCutLine(i + item.width)
     && canPlace(i, item.width, item.height)) {
      front[i + item.width] = cnt + 1;
      prev[i + item.width] = i;
    }
    // Try to place rotated
    if (item.width != item.height
     && i + item.height <= region_.maxX()
     && front[i + item.height] <= cnt
     && utils::fitsMinWaste(item.width, region_.height())
     && isAdmissibleCutLine(i + item.height)
     && canPlace(i, item.height, item.width)) {
      front[i + item.height] = cnt + 1;
      prev[i + item.height] = i;
    }
  }

  reportFront(front, prev);

  // Build the plates
  RowSolution solution(region_);
  int cur = region_.maxX();
  while (cur != 0) {
    int x = prev[cur];
    if (front[cur] != front[x]) {
      assert (front[x] + 1 ==  front[cur]);
      Item item = sequence_[front[x]];
      int width, height;
      if (cur - x == item.width) {
        width = item.width;
        height = item.height;
      }
      else {
        assert (cur - x == item.height);
        width = item.height;
        height = item.width;
      }
      if (canPlaceDown(x, width, height)) {
        Rectangle place = Rectangle::FromCoordinates(x, region_.minY(), x + width, region_.minY() + height);
        solution.items.emplace_back(place, item.id);
      }
      else {
        assert (canPlaceUp(x, width, height));
        Rectangle place = Rectangle::FromCoordinates(x, region_.maxY() - height, x + width, region_.maxY());
        solution.items.emplace_back(place, item.id);
      }
    }
    cur = x;
  }
  reverse(solution.items.begin(), solution.items.end());
  return solution;
}

RowSolution RowPacker::runDiagnostic() {
  RowSolution approximate = runApproximate();
  RowSolution exact = runExact();
  if (approximate.nItems() != exact.nItems()) {
    cout << "Exact row algorithm obtains " << exact.nItems() << " items but approximate one obtains " << approximate.nItems() << endl;
    cout << "Exact" << endl;
    exact.report();
    cout << "Approximate" << endl;
    approximate.report();
    cout << endl;
  }
  return exact;
}

void RowPacker::reportFront(const std::vector<int> &front, const std::vector<int> &prev) const {
  if (!options_.tracePackingFronts) return;
  vector<int> changes = extractFrontChanges(front);
  cout << "Front changes for row: " << changes.size() << endl;
}

void RowPacker::fillXData(RowDescription &description, int maxUsedX) const {
  description.maxUsedX = firstValidVerticalCut(maxUsedX, true /* always assumed tight */);
  description.tightX = description.maxUsedX == maxUsedX;
}

void RowPacker::fillYData(RowDescription &description) const {
  int maxHeight = 0;
  for (int i = start_; i < start_ + description.nItems; ++i) {
    maxHeight = max(heights_[i], maxHeight);
    assert (utils::fitsMinWaste(heights_[i], region_.height()));
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
  int firstCut = firstValidHorizontalCut(description.maxUsedY, description.tightY);
  if (firstCut != description.maxUsedY) {
    description.maxUsedY = firstCut;
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

bool RowPacker::canPlace(int x, int width, int height) {
  return canPlaceDown(x, width, height)
      || canPlaceUp(x, width, height);
}

bool RowPacker::canPlaceDown(int x, int width, int height) {
  Rectangle place = Rectangle::FromCoordinates(x, region_.minY(), x + width, region_.minY() + height);
  for (Defect d : defects_) {
    if (d.intersects(place))
      return false;
  }
  return true;
}

bool RowPacker::canPlaceUp(int x, int width, int height) {
  Rectangle place = Rectangle::FromCoordinates(x, region_.maxY() - height, x + width, region_.maxY());
  for (Defect d : defects_) {
    if (d.intersects(place))
      return false;
  }
  return true;
}

bool RowPacker::isAdmissibleCutLine(int x) const {
  if (x == region_.minX() || x == region_.maxX())
    return true;
  if (x < region_.minX() + Params::minWaste || x > region_.maxX() - Params::minWaste)
    return false;
  for (Defect d : defects_) {
    if (d.intersectsVerticalLine(x))
      return false;
  }
  return true;
}

