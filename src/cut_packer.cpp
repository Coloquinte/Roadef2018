
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

CutPacker::CutDescription CutPacker::count(Rectangle cut, int start, const std::vector<Defect> &defects) {
  runCommon(cut, start, defects);
  return countBacktrack();
}

void CutPacker::runCommon(Rectangle cut, int start, const std::vector<Defect> &defects) {
  init(cut, start, defects);
  checkConsistency();
  sort(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxY() < b.maxY();
        });
  assert (region_.minY() == 0);

  front_.clear();
  front_.init(region_.minY(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    propagate(i, front_[i].end);
    propagateBreakpoints(i);
  }
  front_.checkConsistency();
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
    if (result.nItems > 0 && utils::fitsMinWaste(result.maxUsedY, region_.maxY()))
      front_.insert(beginCoord, result.maxUsedY, previousItems + result.nItems, previousFront);
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
    if (isAdmissibleCutLine(bp) && bp >= region_.minY() + Params::minYY) {
      propagate(maxValid, bp);
    }
  }
}

void CutPacker::buildSlices() {
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

CutSolution CutPacker::backtrack() {
  buildSlices();
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
  buildSlices();
  CutDescription description;
  std::vector<RowPacker::RowDescription> rows;
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
  for (Defect d : defects_) {
    if (d.intersectsHorizontalLine(y))
      return false;
  }
  return true;
}

