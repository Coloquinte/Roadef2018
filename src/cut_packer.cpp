
#include "cut_packer.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

CutPacker::CutPacker(const Problem &problem, const vector<Item> &sequence)
: Packer(problem, sequence)
, rowPacker_(problem, sequence) {
}

CutSolution CutPacker::run(Rectangle cut, int start, const std::vector<Defect> &defects) {
  runCommon(cut, start, defects);
  return backtrack();
}

int CutPacker::count(Rectangle cut, int start, const std::vector<Defect> &defects) {
  runCommon(cut, start, defects);
  return countBacktrack();
}

void CutPacker::runCommon(Rectangle cut, int start, const std::vector<Defect> &defects) {
  init(cut, start, defects);
  sort(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxY() < b.maxY();
        });
  assert (region_.minY() == 0);

  front_.clear();
  front_.init(region_.minY(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    auto elt = front_[i];
    propagate(i, elt.valeur, elt.end);
    propagateBreakpoints(i);
  }
  front_.checkConsistency();
}

RowPacker::Quality CutPacker::countRow(int start, int minY, int maxY) {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return rowPacker_.count(row, start, defects_);
}

RowSolution CutPacker::packRow(int start, int minY, int maxY) {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return rowPacker_.run(row, start, defects_);
}

void CutPacker::propagate(int previousFront, int previousItems, int beginCoord) {
  // TODO: more precise definition for domination
  for (int endCoord = region_.maxY(); endCoord >= beginCoord + minYY_; --endCoord) {
    RowPacker::Quality result = countRow(previousItems, beginCoord, endCoord);

    // We cut all solutions with maxUsed + minWaste_ <= end
    int maxUsed = result.maxUsedY;
    if (maxUsed + minWaste_ < endCoord) {
      // Shortcut from the current solution: no need to try all the next ones
      endCoord = maxUsed + minWaste_;
      result = countRow(previousItems, beginCoord, endCoord);
    }
    front_.insert(beginCoord, endCoord, previousItems + result.nItems, previousFront);

    if (maxUsed < endCoord) {
      // Try the fully packed case too
      result = countRow(previousItems, beginCoord, maxUsed);
      front_.insert(beginCoord, maxUsed, previousItems + result.nItems, previousFront);
    }
  }
}

void CutPacker::propagateBreakpoints(int after) {
  int from = front_[after].end;
  assert (is_sorted(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxY() < b.maxY();
        }));
  for (const Defect &defect : defects_) {
    int bp = defect.maxY() + 1;
    if (bp <= from)
      continue;
    if (after + 1 < front_.size() && bp >= front_[after+1].end)
      continue;
    // Find the previous front element we can extend
    int prev = 0;
    for (; prev < front_.size(); ++prev) {
      // Can we extend the previous row?
      if (front_[prev].end + minWaste_ > bp)
        break;
      // Can we create a row before?
      if (prev == 0 && front_[prev].end + minYY_ > bp)
        break;
    }
    --prev;
    if (prev < 0)
      continue;
    assert (prev <= after);

    // Propagate from here
    propagate(prev, front_[prev].valeur, bp);
  }
}

CutSolution CutPacker::backtrack() {
  slices_.clear();
  slices_.push_back(region_.maxY());

  int cur = front_.size() - 1;
  while (cur != 0) {
    auto elt = front_[cur];
    if (elt.begin + minYY_ > slices_.back())
      continue;
    assert (elt.previous < cur);
    assert (elt.begin < slices_.back());
    slices_.push_back(elt.begin);
    cur = elt.previous;
  }
  if (slices_.back() != region_.minY())
    slices_.push_back(region_.minY());
  reverse(slices_.begin(), slices_.end());

  int nPacked = start_;
  CutSolution cutSolution(region_);
  for (size_t i = 0; i + 1 < slices_.size(); ++i) {
    RowSolution solution = packRow(nPacked, slices_[i], slices_[i+1]);
    nPacked += solution.nItems();
    assert (solution.height() >= minYY_);
    cutSolution.rows.push_back(solution);
  }

  return cutSolution;
}

int CutPacker::countBacktrack() {
  int cur = front_.size() - 1;
  while (cur != 0) {
    auto elt = front_[cur];
    if (elt.begin + minYY_ > slices_.back())
      continue;
    assert (elt.previous < cur);
    cur = elt.previous;
  }
  return front_[cur].valeur;
}

