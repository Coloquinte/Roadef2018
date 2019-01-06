
#include "row_merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;


RowMerger::RowMerger(SolverParams options, const pair<vector<Item>, vector<Item> > &sequences)
: Merger(options, sequences) {
}

void RowMerger::init(Rectangle row, const vector<Defect> &defects, pair<int, int> starts) {
  vector<pair<int, int> > vec;
  vec.push_back(starts);
  init(row, defects, vec);
}

void RowMerger::init(Rectangle row, const vector<Defect> &defects, const vector<pair<int, int> > &starts) {
  Merger::init(row, defects, starts, row.minX());
}

void RowMerger::buildFront() {
  if (options_.rowMerging == PackingOption::Approximate) {
    buildFrontApproximate();
  }
  else {
    buildFrontExact();
  }
}

void RowMerger::buildFrontApproximate() {
  // TODO: quick handling of the trivial case
  for (int i = 0; i < (int) front_.size(); ++i) {
    insertIntermediateDefects(i);
    propagateFromElement(i);
  }
  propagateFrontToEnd();
  checkConsistency();
}

void RowMerger::insertIntermediateDefects(int i) {
  // TODO: allow some waste after the defect
  if (i == 0)
    return;
  int minCoord = front_[i-1].coord;
  int maxCoord = front_[i].coord;
  bool found = false;
  int defectCoord = maxCoord;
  for (Defect d : defects_) {
    int coord = d.maxX() + 1;
    if (coord <= minCoord) continue;
    if (coord >= maxCoord) continue;
    if (!found || coord < defectCoord) {
      found = true;
      defectCoord = coord;
    }
  }
  if (!found)
    return;
  // TODO: fix this automatically
  if (!isAdmissibleCutLine(defectCoord))
    return;

  // Gather previous elements and eliminate duplicates
  vector<FrontElement> candidates;
  for (int j = 0; j < i; ++j) {
    FrontElement elt = front_[j];
    if (elt.coord + Params::minWaste >= defectCoord)
      continue;
    candidates.emplace_back(defectCoord, j, elt.n);
  }
  for (size_t j = 0; j < candidates.size();) {
    bool dominated = false;
    for (size_t k = 0; k < candidates.size(); ++k) {
      if (j == k) continue;
      auto elt1 = candidates[j].n;
      auto elt2 = candidates[k].n;
      if (elt2.first >= elt1.first && elt2.second >= elt1.second)
        dominated = true;
    }
    if (dominated) {
      candidates.erase(candidates.begin() + j);
    }
    else {
      ++j;
    }
  }
  for (FrontElement elt : candidates) {
    assert (elt.prev < i);
  }
  front_.insert(front_.begin() + i, candidates.begin(), candidates.end());
}

void RowMerger::propagateFromElement(int i) {
  FrontElement elt = front_[i];
  int x = elt.coord;
  if (elt.n.first < (int) sequences_.first.size()) {
    Item item = sequences_.first[elt.n.first];
    if (canPlace(x, item.width, item.height))
      insertFrontCleanup(x + item.width , i, elt.n.first + 1, elt.n.second, Params::minWaste);
    if (canPlace(x, item.height, item.width))
      insertFrontCleanup(x + item.height, i, elt.n.first + 1, elt.n.second, Params::minWaste);
  }
  if (elt.n.second < (int) sequences_.second.size()) {
    Item item = sequences_.second[elt.n.second];
    if (canPlace(x, item.width, item.height))
      insertFrontCleanup(x + item.width , i, elt.n.first, elt.n.second + 1, Params::minWaste);
    if (canPlace(x, item.height, item.width))
      insertFrontCleanup(x + item.height, i, elt.n.first, elt.n.second + 1, Params::minWaste);
  }
}

void RowMerger::propagateFrontToEnd() {
  int origFrontSize = front_.size();
  for (int i = 0; i < origFrontSize; ++i) {
    FrontElement elt = front_[i];
    if (elt.coord > region_.maxX() - Params::minWaste)
      continue;
    front_.emplace_back(region_.maxX(), i, front_[i].n);
  }
}

void RowMerger::buildFrontExact() {
  // TODO
}

vector<pair<int, int> > RowMerger::getParetoFront() const {
  return Merger::getParetoFront(region_.maxX());
}

vector<int> RowMerger::getUsableItemWidths(const vector<Item> &sequence, int start) const {
  vector<int> widths;
  int maxWidth = region_.width();
  int maxHeight = region_.height();
  int totWidth = 0;
  for (int i = start; i < (int) sequence.size(); ++i) {
    Item item = sequence[i];
    int width;
    if (utils::fitsMinWaste(item.height, maxHeight))
      width = item.width;
    else if(utils::fitsMinWaste(item.width, region_.height()))
      width = item.height;
    else
      break;

    totWidth += width;
    if (totWidth > maxWidth)
      break;
    widths.push_back(width);
  }
  return widths;
}

vector<pair<int, int> > RowMerger::optimisticParetoFront() const {
  vector<pair<int, int> > ret;
  int maxWidth = region_.width();
  for (pair<int, int> start : starts_) {
    pair<vector<int>, vector<int> > widths;
    widths.first = getUsableItemWidths(sequences_.first, start.first);
    widths.second = getUsableItemWidths(sequences_.second, start.second);

    pair<int, int> totWidth;
    totWidth.first = 0;
    totWidth.second = accumulate(widths.second.begin(), widths.second.end(), 0);
    size_t j = widths.second.size();
    for (size_t i = 0; i <= widths.first.size(); ++i) {
      for (; totWidth.first + totWidth.second < maxWidth && j > 0; --j) {
        totWidth.second -= widths.second[j-1];
      }
      ret.emplace_back( start.first + (int) i, start.second + (int) j );
    }
  }
  keepOnlyNonDominated(ret);
  return ret;
}

RowSolution RowMerger::getSolution(pair<int, int> ends) const {
  RowSolution solution(region_);
  // Find the corresponding element on the front
  int cur = -1;
  for (int i = 0; i < (int) front_.size(); ++i) {
    FrontElement elt = front_[i];
    if (elt.coord == region_.maxX() && elt.n == ends)
      cur = i;
  }
  assert (cur >= 0);
  // Backtrack
  while (true) {
    FrontElement curElt = front_[cur];
    int prev = curElt.prev;
    if (prev < 0)
      break;
    FrontElement prevElt = front_[prev];
    if (curElt.n != prevElt.n) {
      // An item needs to be placed here
      Item item = curElt.n.first != prevElt.n.first ?
        sequences_.first[prevElt.n.first]
      : sequences_.second[prevElt.n.second];
      // Now decide how to place it
      int width = curElt.coord - prevElt.coord;
      assert (item.width == width || item.height == width);
      int height = item.width ^ item.height ^ width;
      int x = prevElt.coord;
      assert (isAdmissibleCutLine(x));
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
    cur = prev;
  }
  reverse(solution.items.begin(), solution.items.end());
  checkSolution(solution);
  return solution;
}

void RowMerger::checkSolution(const RowSolution &row) const {
  if (row.items.empty()) return;

  for (const ItemSolution &item : row.items) {
    assert (row.contains(item));
    assert (utils::fitsMinWaste(row.minY(), item.minY()));
    assert (utils::fitsMinWaste(item.maxY(), row.maxY()));
    assert (item.maxY() == row.maxY() || item.minY() == row.minY());
    assert (isAdmissibleCutLine(item.minX()));
    assert (isAdmissibleCutLine(item.maxX()));
  }

  assert (utils::fitsMinWaste(row.minX(), row.items.front().minX()));
  assert (utils::fitsMinWaste(row.items.back().maxX(), row.maxX()));

  for (int i = 0; i+1 < (int) row.items.size(); ++i) {
    ItemSolution item1 = row.items[i];
    ItemSolution item2 = row.items[i+1];
    assert (item1.maxX() <= item2.minX());
    assert (utils::fitsMinWaste(item1.maxX(), item2.minX()));
  }
}

bool RowMerger::canPlace(int x, int width, int height) const {
  return canPlaceDown(x, width, height) || canPlaceUp(x, width, height);
}

bool RowMerger::canFit(int x, int width, int height) const {
  if (!utils::fitsMinWaste(height, region_.height()))
    return false;
  if (!utils::fitsMinWaste(x + width, region_.maxX()))
    return false;
  if (!isAdmissibleCutLine(x + width))
    return false;
  return true;
}
bool RowMerger::canPlaceDown(int x, int width, int height) const {
  if (!canFit(x, width, height))
    return false;
  // Attempt to place at the botton of the region
  Rectangle place = Rectangle::FromCoordinates(x, region_.minY(), x + width, region_.minY() + height);
  for (Defect d : defects_) {
    if (d.intersects(place))
      return false;
  }
  return true;
}

bool RowMerger::canPlaceUp(int x, int width, int height) const {
  if (!canFit(x, width, height))
    return false;
  // Attempt to place at the top of the region
  Rectangle place = Rectangle::FromCoordinates(x, region_.maxY() - height, x + width, region_.maxY());
  for (Defect d : defects_) {
    if (d.intersects(place))
      return false;
  }
  return true;
}

bool RowMerger::isAdmissibleCutLine(int x) const {
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

void RowMerger::checkConsistency() const {
  Merger::checkConsistency();
  for (FrontElement elt : front_) {
    assert (isAdmissibleCutLine(elt.coord));
    assert ( (elt.coord == region_.minX()) == (elt.prev == -1));
    if (elt.prev != - 1) {
      FrontElement prev = front_[elt.prev];
      assert ( (prev.n.first == elt.n.first && prev.n.second + 1 == elt.n.second)
            || (prev.n.first + 1 == elt.n.first && prev.n.second == elt.n.second)
            || (prev.n.first == elt.n.first && prev.n.second == elt.n.second));
    }
  }
}
 

