
#include "merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

using namespace std;

void Merger::init(Rectangle region, const vector<Defect> &defects, const vector<pair<int, int> > &starts, int coord) {
  region_ = region;
  defects_.clear();
  for (Defect d : defects) {
    if (d.intersects(region_))
      defects_.push_back(d);
  }
  front_.clear();
  for (pair<int, int> start : starts) {
    front_.emplace_back(coord, -1, start);
  }
}

void Merger::checkConsistency() const {
  for (size_t i = 0; i < front_.size(); ++i) {
    FrontElement elt = front_[i];
    assert (elt.n.first  <= (int) sequences_.first.size());
    assert (elt.n.second <= (int) sequences_.second.size());
    if (elt.prev == -1) continue;
    assert (elt.prev >= 0 && elt.prev < (int) i);
    FrontElement prev = front_[elt.prev];
    assert (prev.coord < elt.coord);
    assert ( (prev.n.first == elt.n.first && prev.n.second + 1 == elt.n.second)
          || (prev.n.first == elt.n.first + 1 && prev.n.second == elt.n.second)
          || (prev.n.first == elt.n.first && prev.n.second == elt.n.second));
  }
}

void Merger::eraseDominated(int coord, int nFirst, int nSecond, int distance) {
  auto it = remove_if(front_.begin(), front_.end(),
      [=](const FrontElement &elt) {
        return (coord + distance < elt.coord || coord == elt.coord)
                && nFirst >= elt.n.first && nSecond >= elt.n.second;
      });
  front_.erase(it, front_.end());
}

bool Merger::isDominated(int coord, int nFirst, int nSecond, int distance) {
  for (FrontElement elt : front_) {
    if ((elt.coord + distance < coord || elt.coord == coord)
        && elt.n.first >= nFirst && elt.n.second >= nSecond)
      return true;
  }
  return false;
}

void Merger::insertFront(int coord, int prev, int nFirst, int nSecond) {
  // Find where to insert
  auto insertPoint = front_.begin();
  for (; insertPoint != front_.end(); ++insertPoint) {
    if (insertPoint->coord > coord) break;
  }
  // Insert at the right place
  front_.insert(insertPoint, FrontElement(coord, prev, make_pair(nFirst, nSecond)));
  // TODO: update prev for all later elements; not necessary for now
}

void Merger::insertFrontCleanup(int coord, int prev, int nFirst, int nSecond, int distance) {
  if (isDominated(coord, nFirst, nSecond, distance))
    return;
  eraseDominated(coord, nFirst, nSecond, distance);
  insertFront(coord, prev, nFirst, nSecond);
}

void Merger::keepOnlyNonDominated(vector<pair<int, int> > &front) {
  // Filter dominated elements out
  for (int i = 0; i < (int) front.size();) {
    bool dominated = false;
    pair<int, int> p1 = front[i];
    for (int j = 0; j < (int) front.size(); ++j) {
      if (i == j) continue;
      pair<int, int> p2 = front[j];
      if (p2.first >= p1.first && p2.second >= p1.second)
        dominated = true;
    }
    if (dominated) {
      front.erase(front.cbegin() + i);
    }
    else {
      ++i;
    }
  }

  sort(front.begin(), front.end());
}

vector<pair<int, int> > Merger::getParetoFront(int coord) const {
  vector<pair<int, int> > paretoFront;
  for (FrontElement elt : front_) {
    if (elt.coord == coord)
      paretoFront.push_back(elt.n);
  }

  keepOnlyNonDominated(paretoFront);
  return paretoFront;
}

