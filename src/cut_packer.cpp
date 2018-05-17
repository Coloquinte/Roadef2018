
#include "cut_packer.hpp"
#include "pareto_front.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

CutPacker::CutPacker(const Problem &problem, const vector<Item> &sequence)
: Packer(problem, sequence)
, rowPacker_(problem, sequence) {
}

CutSolution CutPacker::run(Rectangle cut, int start, const std::vector<Defect> &defects) {
  init(cut, start, defects);
  // Dynamic programming on the rows i.e. second-level cuts
  assert (region_.minY() == 0);

  front_.clear();
  front_.init(region_.minY(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    auto elt = front_[i];
    propagate(i, elt.valeur, elt.end);
    propagateBreakpoints(i);
  }
  front_.checkConsistency();

  return backtrack();
}

int CutPacker::count(Rectangle cut, int start, const std::vector<Defect> &defects) {
  return run(cut, start, defects).nItems();
}

RowPacker::Quality CutPacker::countRow(int start, int minY, int maxY) {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return rowPacker_.count(row, start, defects_);
}

RowSolution CutPacker::packRow(int start, int minY, int maxY) {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return rowPacker_.run(row, start, defects_);
}

vector<int> CutPacker::computeBreakpoints() const {
  vector<int> ret;
  for (const Defect &defect : defects_) {
    if (!defect.intersects(region_))
      continue;
    ret.push_back(defect.maxY() + 1);
  }
  sort(ret.begin(), ret.end());
  return ret;
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
  for (int bp : computeBreakpoints()) {
    if (bp <= from)
      continue;
    if (after + 1 < front_.size() && bp >= front_[after+1].end)
      break;
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
  vector<int> slices;
  slices.push_back(region_.maxY());

  int cur = front_.size() - 1;
  while (cur != 0) {
    auto elt = front_[cur];
    if (elt.begin + minYY_ > slices.back())
      continue;
    slices.push_back(elt.begin);
    cur = elt.previous;
  }
  if (slices.back() != region_.minY())
    slices.push_back(region_.minY());
  reverse(slices.begin(), slices.end());

  int nPacked = start_;
  CutSolution cutSolution(region_);
  for (size_t i = 0; i + 1 < slices.size(); ++i) {
    RowSolution solution = packRow(nPacked, slices[i], slices[i+1]);
    nPacked += solution.nItems();
    assert (solution.height() >= minYY_);
    cutSolution.rows.push_back(solution);
  }

  return cutSolution;
}

