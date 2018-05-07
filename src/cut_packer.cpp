
#include "cut_packer.hpp"
#include "pareto_front.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

CutSolution CutPacker::run(const Packer &parent, Rectangle cut, int start) {
  CutPacker packer(parent, cut, start);
  return packer.run();
}

int CutPacker::count(const Packer &parent, Rectangle cut, int start) {
  CutPacker packer(parent, cut, start);
  return packer.count();
}

CutPacker::CutPacker(const Packer &parent, Rectangle cut, int start)
: Packer(parent) {
  region_ = cut;
  start_ = start;
}

CutSolution CutPacker::run() {
  // Dynamic programming on the rows i.e. second-level cuts
  assert (region_.minY() == 0);

  front_.init(region_.minY(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    auto elt = front_[i];
    propagate(i, elt.valeur, elt.end);
  }
  front_.checkConsistency();

  return backtrack();
}

int CutPacker::count() {
  return run().nItems();
}

RowPacker::Quality CutPacker::countRow(int start, int minY, int maxY) const {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return RowPacker::count(*this, row, start);
}

RowSolution CutPacker::packRow(int start, int minY, int maxY) const {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return RowPacker::run(*this, row, start);
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

CutSolution CutPacker::backtrack() {
  CutSolution cutSolution(region_);
  int cur = front_.size() - 1;
  bool lastRow = true;
  while (cur != 0) {
    auto elt = front_[cur];
    int next = elt.previous;
    auto prevElt = front_[next];

    int beginCoord = elt.begin;
    int endCoord = lastRow ? region_.maxY() : elt.end;

    auto solution = packRow(prevElt.valeur, beginCoord, endCoord);
    // There is one corner case (minWaste_ at the end depending on orientation) where we catch a better solution with a smaller row
    assert (prevElt.valeur + solution.nItems() == elt.valeur || endCoord != elt.end);
    assert (solution.height() >= minYY_);
    cutSolution.rows.push_back(solution);
    cur = next;
    lastRow = false;
  }
  reverse(cutSolution.rows.begin(), cutSolution.rows.end());

  return cutSolution;

}

