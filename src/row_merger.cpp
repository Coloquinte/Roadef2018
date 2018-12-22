
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
  for (int i = 0; i < (int) front_.size(); ++i) {
    // Propagate from front element i
    FrontElement elt = front_[i];
    int x = elt.coord;
    if (elt.n.first < (int) sequences_.first.size()) {
      Item item = sequences_.first[elt.n.first];
      if (canPlace(x, item.width, item.height)) insertFrontCleanup(x + item.width , i, elt.n.first + 1, elt.n.second);
      if (canPlace(x, item.height, item.width)) insertFrontCleanup(x + item.height, i, elt.n.first + 1, elt.n.second);
    }
    if (elt.n.second < (int) sequences_.second.size()) {
      Item item = sequences_.second[elt.n.second];
      if (canPlace(x, item.width, item.height)) insertFrontCleanup(x + item.width , i, elt.n.first, elt.n.second + 1);
      if (canPlace(x, item.height, item.width)) insertFrontCleanup(x + item.height, i, elt.n.first, elt.n.second + 1);
    }
    // TODO: propagate from defects
  }
}

void RowMerger::buildFrontExact() {
  // TODO
}

vector<pair<int, int> > RowMerger::getParetoFront() const {
  vector<pair<int, int> > paretoFront;
  for (FrontElement elt : front_) {
    if (elt.coord == region_.maxX())
      paretoFront.push_back(elt.n);
  }
  // TODO: filter dominated elements out
  return paretoFront;
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
    assert (item1.maxX() >= item2.minX());
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
  }
}
 

