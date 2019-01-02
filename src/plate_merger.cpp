
#include "plate_merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

PlateMerger::PlateMerger(SolverParams options, const pair<vector<Item>, vector<Item> > &sequences)
: Merger(options, sequences)
, cutMerger_(options, sequences) {
}

void PlateMerger::init(Rectangle plate, const vector<Defect> &defects, pair<int, int> starts) {
  vector<pair<int, int> > vec;
  vec.push_back(starts);
  init(plate, defects, vec);
}

void PlateMerger::init(Rectangle plate, const vector<Defect> &defects, const vector<pair<int, int> > &starts) {
  Merger::init(plate, defects, starts, plate.minY());
}

void PlateMerger::buildFront() {
  if (options_.plateMerging == PackingOption::Approximate) {
    buildFrontApproximate();
  }
  else {
    buildFrontExact();
  }
}

void PlateMerger::buildFrontApproximate() {
  for (int i = 0; i < (int) front_.size(); ++i) {
    propagateFromElement(i);
    // TODO: propagate from defects
  }
  propagateFrontToEnd();
}

void PlateMerger::propagateFrontToEnd() {
  int origFrontSize = front_.size();
  for (int i = 0; i < origFrontSize; ++i) {
    if (front_[i].coord > region_.maxX() - Params::minXX)
      continue;
    int prev = i;
    pair<int, int> n = front_[i].n;
    int coord = front_[i].coord;
    do {
      // Handle the case where we couldn't pack anything in-between
      coord = findCuttingPosition(coord, region_.maxX());
      front_.emplace_back(coord, prev, n);
      prev = front_.size() - 1;
    } while (coord != region_.maxX());
  }
}

void PlateMerger::propagateFromElement(int i) {
  FrontElement elt = front_[i];
  vector<int> candidates = getMaxXCandidates(elt.coord, elt.n);
  for (int endCoord : candidates) {
    if (endCoord - elt.coord < Params::minXX)
      continue;
    if (endCoord - elt.coord > Params::maxXX)
      continue;
    if (!isAdmissibleCutLine(endCoord))
      continue;
    runCutMerger(elt.coord, endCoord, elt.n);
    for (pair<int, int> n : cutMerger_.getParetoFront()) {
      insertFrontCleanup(endCoord, i, n.first, n.second, Params::minWaste);
    }
  }
}

void PlateMerger::buildFrontExact() {
  // TODO
}

void PlateMerger::runCutMerger(int minX, int maxX, pair<int, int> starts) {
  Rectangle cut = Rectangle::FromCoordinates(minX, region_.minY(), maxX, region_.maxY());
  cutMerger_.init(cut, defects_, starts);
  cutMerger_.buildFront();
}

vector<int> PlateMerger::getMaxXCandidates(int minX, pair<int, int> starts) {
  // TODO: better estimation of the maximum number of items that can be packed
  // TODO: factor in a specific function

  int maxArea = region_.area();
  vector<int> candidates;

  int area1 = 0;
  int minDim1 = 0;
  for (int t1 = starts.first; t1 < (int) sequences_.first.size(); ++t1) {
    Item item = sequences_.first[t1];
    area1 += item.area();
    minDim1 = max(minDim1, item.width);
    if (area1 > maxArea) break;
    if (item.height >= minDim1)
      candidates.push_back(minX + item.height);
    if (item.height + Params::minWaste >= minDim1)
      candidates.push_back(minX + item.height + Params::minWaste);
    if (item.width >= minDim1)
      candidates.push_back(minX + item.width);
    if (item.width + Params::minWaste >= minDim1)
      candidates.push_back(minX + item.width  + Params::minWaste);
  }

  int area2 = 0;
  int minDim2 = 0;
  for (int t2 = starts.second; t2 < (int) sequences_.second.size(); ++t2) {
    Item item = sequences_.second[t2];
    area2 += item.area();
    minDim2 = max(minDim2, item.width);
    if (area2 > maxArea) break;
    if (item.height >= minDim2)
      candidates.push_back(minX + item.height);
    if (item.height + Params::minWaste >= minDim2)
      candidates.push_back(minX + item.height + Params::minWaste);
    if (item.width >= minDim2)
      candidates.push_back(minX + item.width);
    if (item.width + Params::minWaste >= minDim2)
      candidates.push_back(minX + item.width  + Params::minWaste);
  }

  candidates.push_back(minX + Params::maxXX);
  candidates.push_back(region_.maxX());

  sort(candidates.begin(), candidates.end());
  candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end());

  // TODO: make this smarter (multiple items)
  // TODO: take defects into account
  return candidates;
}

vector<pair<int, int> > PlateMerger::getParetoFront(bool useAll) const {
  if (useAll)
    return Merger::getParetoFront(region_.maxX());
  // Check whether we have some residual left
  for (FrontElement elt : front_) {
    if (elt.n.first  == (int) sequences_.first.size()
     && elt.n.second == (int) sequences_.second.size()) {
      vector<pair<int, int> > ret;
      ret.push_back(elt.n);
      return ret;
    }
  }
  return Merger::getParetoFront(region_.maxX());
}

int PlateMerger::getEndFrontPos(std::pair<int, int> ends, bool useAll) const {
  int cur = -1;
  bool canStopEarly = !useAll
    && ends.first  == (int) sequences_.first.size()
    && ends.second == (int) sequences_.second.size();
  for (int i = 0; i < (int) front_.size(); ++i) {
    FrontElement elt = front_[i];
    if (canStopEarly) {
      if(cur == -1 && elt.n == ends)
        cur = i;
    }
    else {
      if (elt.coord == region_.maxX() && elt.n == ends)
        cur = i;
    }
  }

  assert (cur >= 0);
  return cur;
}

PlateSolution PlateMerger::getSolution(pair<int, int> ends, bool useAll) {
  PlateSolution solution(region_);
  // Find the corresponding element on the front
  int cur = getEndFrontPos(ends, useAll);

  // Backtrack
  while (true) {
    FrontElement curElt = front_[cur];
    int prev = curElt.prev;
    if (prev < 0)
      break;
    FrontElement prevElt = front_[prev];
    runCutMerger(prevElt.coord, curElt.coord, prevElt.n);
    CutSolution cut = cutMerger_.getSolution(curElt.n);
    solution.cuts.push_back(cut);
    cur = prev;
  }
  reverse(solution.cuts.begin(), solution.cuts.end());
  checkSolution(solution);
  return solution;
}

std::pair<int, int> PlateMerger::getStarts(std::pair<int, int> ends, bool useAll) const {
  // Find the corresponding element on the front
  int cur = getEndFrontPos(ends, useAll);

  // Backtrack
  while (true) {
    FrontElement curElt = front_[cur];
    cur = curElt.prev;
    if (cur < 0)
      return curElt.n;
  }
}

void PlateMerger::checkSolution(const PlateSolution &plate) const {
  if (plate.cuts.empty()) return;

  for (const CutSolution &cut: plate.cuts) {
    assert (plate.contains(cut));
    assert (cut.maxY() == plate.maxY() && cut.minY() == plate.minY());
    assert (isAdmissibleCutLine(cut.minX()));
    assert (isAdmissibleCutLine(cut.maxX()));
  }

  assert (plate.minX() == plate.cuts.front().minX());

  for (int i = 0; i+1 < (int) plate.cuts.size(); ++i) {
    CutSolution cut1 = plate.cuts[i];
    CutSolution cut2 = plate.cuts[i+1];
    assert (cut1.maxX() == cut2.minX());
  }
}

bool PlateMerger::isAdmissibleCutLine(int x) const {
  if (x == 0 || x == Params::widthPlates)
    return true;
  if (x < Params::minXX || x > Params::widthPlates - Params::minWaste)
    return false;
  for (Defect d : defects_) {
    if (d.intersectsVerticalLine(x))
      return false;
  }
  return true;
}

int PlateMerger::findCuttingPosition(int from, int to) const {
  if (to - from <= Params::maxXX)
    return to;
  int pos = min(to - Params::minXX, from + Params::maxXX);
  while (!isAdmissibleCutLine(pos)) {
    --pos;
  }
  return pos;
}

void PlateMerger::checkConsistency() const {
  Merger::checkConsistency();
  for (FrontElement elt : front_) {
    assert (isAdmissibleCutLine(elt.coord));
    assert ( (elt.coord == region_.minX()) == (elt.prev == -1));
  }
}


