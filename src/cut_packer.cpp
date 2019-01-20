// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#include "cut_packer.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

using namespace std;

CutPacker::CutPacker(const vector<Item> &sequence, SolverParams options)
: Packer(sequence, options)
, rowPacker_(sequence, options) {
}

CutSolution CutPacker::run(Rectangle cut, int start, const vector<Defect> &defects) {
  setup(cut, start, defects);
  if (options_.cutPacking == PackingOption::Approximate) {
    return runApproximate();
  }
  else if (options_.cutPacking == PackingOption::Exact) {
    return runExact();
  }
  else {
    return runDiagnostic();
  }
}

CutPacker::CutDescription CutPacker::count(Rectangle cut, int start, const vector<Defect> &defects) {
  setup(cut, start, defects);
  if (options_.cutPacking == PackingOption::Approximate) {
    return countApproximate();
  }
  else if (options_.cutPacking == PackingOption::Exact) {
    return countExact();
  }
  else {
    return countDiagnostic();
  }
}

CutSolution CutPacker::runApproximate() {
  commonApproximate();
  return backtrack();
}

CutSolution CutPacker::runExact() {
  commonExact();
  return backtrack();
}

CutSolution CutPacker::runDiagnostic() {
  CutSolution approximate = runApproximate();
  CutSolution exact = runExact();
  if (approximate.nItems() != exact.nItems()) {
    cout << "Exact cut algorithm obtains " << exact.nItems() << " items but approximate one obtains " << approximate.nItems() << endl;
    cout << "Exact" << endl;
    exact.report();
    cout << "Approximate" << endl;
    approximate.report();
    cout << endl;
  }
  return exact;
}

void CutPacker::reportFront(const std::vector<int> &front, const std::vector<int> &prev) const {
  if (!options_.tracePackingFronts) return;
  vector<int> changes = extractFrontChanges(front);
  cout << "Front changes for cut: " << changes.size() << endl;
}

CutPacker::CutDescription CutPacker::countApproximate() {
  commonApproximate();
  return countBacktrack();
}

CutPacker::CutDescription CutPacker::countExact() {
  commonExact();
  return countBacktrack();
}

CutPacker::CutDescription CutPacker::countDiagnostic() {
  CutDescription approximate = countApproximate();
  CutDescription exact = countExact();
  if (approximate.nItems != exact.nItems) {
    cout << "Exact cut algorithm obtains " << exact.nItems << " items but approximate one obtains " << approximate.nItems << endl;
  }
  return exact;
}

void CutPacker::setup(Rectangle cut, int start, const vector<Defect> &defects) {
  init(cut, start, defects);
  checkConsistency();
  sort(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxY() < b.maxY();
        });
  assert (region_.minY() == 0);
  assert (region_.maxY() == Params::heightPlates);
}

void CutPacker::commonApproximate() {
  // Fill the front
  front_.clear();
  front_.init(region_.minY(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    propagate(i, front_[i].end);
    propagateBreakpoints(i);
  }
  front_.checkConsistency();

  // Build the slices
  slices_.clear();
  slices_.push_back(region_.maxY());

  int cur = front_.size() - 1;
  while (cur != 0) {
    auto elt = front_[cur];
    if (elt.begin + Params::minYY > slices_.back())
      continue;
    assert (elt.previous < cur);
    assert (elt.begin < slices_.back());
    slices_.push_back(elt.begin);
    cur = elt.previous;
  }
  if (slices_.back() != region_.minY())
    slices_.push_back(region_.minY());
  reverse(slices_.begin(), slices_.end());
}

void CutPacker::commonExact() {
  // Fill the front
  vector<int> front(Params::heightPlates + 1, -1);
  vector<int> prev(Params::heightPlates + 1, -1);
  front[0] = start_;
  for (int j = Params::minYY; j <= Params::heightPlates; ++j) {
    int best = -1;
    int pred = -1;
    if (!isAdmissibleCutLine(j)) continue;
    if (j != Params::heightPlates && j > Params::heightPlates - Params::minYY) continue;
    for (int i = 0; i <= j - Params::minYY; ++i) {
      if (front[i] < 0) continue;
      int cnt = front[i] + countRow(front[i], i, j).nItems;
      if (cnt > best) {
        best = cnt;
        pred = i;
      }
      front[j] = best;
      prev[j] = pred;
    }
  }

  reportFront(front, prev);

  // Build the slices
  int cur = Params::heightPlates;
  slices_.clear();
  slices_.push_back(Params::heightPlates);
  while (cur != 0) {
    cur = prev[cur];
    slices_.push_back(cur);
  }
  reverse(slices_.begin(), slices_.end());
}

RowPacker::RowDescription CutPacker::countRow(int start, int minY, int maxY) {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return rowPacker_.count(row, start, defects_);
}

RowSolution CutPacker::packRow(int start, int minY, int maxY) {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return rowPacker_.run(row, start, defects_);
}

void CutPacker::propagate(int previousFront, int beginCoord) {
  int previousItems = front_[previousFront].value;
  for (int endCoord = region_.maxY() + Params::minWaste; endCoord >= beginCoord + Params::minYY; --endCoord) {
    if (!isAdmissibleCutLine(endCoord)) continue;
    RowPacker::RowDescription result = countRow(previousItems, beginCoord, endCoord);
    if (utils::fitsMinWaste(result.maxUsedY, result.tightY, region_.maxY())) {
      int coord = utils::extendToFit(result.maxUsedY, region_.maxY(), Params::minYY);
      front_.insert(beginCoord, coord, previousItems + result.nItems, previousFront);
    }
    if (!result.tightY
     && isAdmissibleCutLine(result.maxUsedY - Params::minWaste)
     && result.maxUsedY - Params::minWaste >= beginCoord + Params::minYY) {
      RowPacker::RowDescription tight = countRow(previousItems, beginCoord, result.maxUsedY - Params::minWaste);
      if (utils::fitsMinWaste(tight.maxUsedY, tight.tightY, region_.maxY())) {
        int coord = utils::extendToFit(tight.maxUsedY, region_.maxY(), Params::minYY);
        front_.insert(beginCoord, coord, previousItems + tight.nItems, previousFront);
      }
    }
    endCoord = min(endCoord, result.tightY ? result.maxUsedY + Params::minWaste : result.maxUsedY);
  }
}

void CutPacker::propagateBreakpoints(int after) {
  int from = front_[after].end;
  int to = after + 1 < front_.size() ? front_[after+1].end : region_.maxY();
  assert (is_sorted(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxY() < b.maxY();
        }));
  for (const Defect &defect : defects_) {
    int bp = defect.maxY() + 1;
    // Only consider defects that are in between the current front elements
    if (bp <= from)
      continue;
    if (bp >= to)
      continue;
    // Find the previous front elements we can extend, by cutting as early as possible after the defect
    //    * minWaste after the maxUsedY for a row
    //    * minYY after the start of the region
    //    * right after the defect
    // Note that whenever a cut is not admissible, another defect will account for a valid one (as long as it's not tight)
    for (int i = 1; i <= after; ++i) {
      int cutPos = front_[i].end + Params::minWaste;
      if (cutPos > bp && isAdmissibleCutLine(cutPos))
        propagate(i, cutPos);
    }
    int firstCutPos = region_.minY() + Params::minYY;
    if (bp < firstCutPos && isAdmissibleCutLine(firstCutPos)) {
      propagate(0, firstCutPos);
    }
    int maxValid = 0;
    for (int i = 1; i <= after; ++i) {
      if (front_[i].end + Params::minWaste <= bp)
        maxValid = i;
    }
    if (isAdmissibleCutLine(bp)) {
      propagate(maxValid, bp);
    }
  }
}

CutSolution CutPacker::backtrack() {
  int nPacked = start_;
  CutSolution cutSolution(region_);
  for (size_t i = 0; i + 1 < slices_.size(); ++i) {
    RowSolution solution = packRow(nPacked, slices_[i], slices_[i+1]);
    nPacked += solution.nItems();
    assert (solution.height() >= Params::minYY);
    cutSolution.rows.push_back(solution);
  }
  assert (cutSolution.nItems() == countBacktrack().nItems);
  return cutSolution;
}

CutPacker::CutDescription CutPacker::countBacktrack() {
  CutDescription description;
  vector<RowPacker::RowDescription> rows;
  for (size_t i = 0; i + 1 < slices_.size(); ++i) {
    RowPacker::RowDescription row = countRow(start_ + description.nItems, slices_[i], slices_[i+1]);
    rows.push_back(row);
    description.nItems += row.nItems;
  }
  description.maxUsedX = region_.minX();
  for (RowPacker::RowDescription row : rows) {
    description.maxUsedX = max(description.maxUsedX, row.maxUsedX);
  }
  description.tightX = true;
  // TODO: handle tightness cases that are not taken into account yet
  for (RowPacker::RowDescription row : rows) {
    if (row.maxUsedX == description.maxUsedX)
      continue;
    if (row.maxUsedX + Params::minWaste > description.maxUsedX)
      description.tightX = false;
  }
  if (!description.tightX) description.maxUsedX += Params::minWaste;
  int firstCut = firstValidVerticalCut(description.maxUsedX, description.tightX);
  if (firstCut != description.maxUsedX) {
    description.maxUsedX = firstCut;
    description.tightX = false;
  }
  return description;
}

bool CutPacker::isAdmissibleCutLine(int y) const {
  if (y == 0 || y == Params::heightPlates)
    return true;
  if (y < Params::minYY || y > Params::heightPlates - Params::minYY)
    return false;
  for (Defect d : defects_) {
    if (d.intersectsHorizontalLine(y))
      return false;
  }
  return true;
}

