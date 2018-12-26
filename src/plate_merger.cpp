
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
    // Propagate from front element i
    FrontElement elt = front_[i];
    vector<int> candidates = getMaxXCandidates(elt.coord, elt.n);
    for (int endCoord : candidates) {
      if (endCoord - elt.coord < Params::minXX)
        continue;
      if (!isAdmissibleCutLine(endCoord))
        continue;
      runCutMerger(elt.coord, endCoord, elt.n);
      for (pair<int, int> n : cutMerger_.getParetoFront()) {
        insertFrontCleanup(endCoord, i, n.first, n.second, Params::minXX);
      }
    }
    // TODO: propagate from defects
  }
  propagateFrontToEnd();
}

void PlateMerger::propagateFrontToEnd() {
  int origFrontSize = front_.size();
  for (int i = 0; i < origFrontSize; ++i) {
    if (front_[i].coord > region_.maxX() - Params::minXX)
      continue;
    if (front_[i].coord < region_.maxX() - Params::maxXX)
      continue;
    front_.emplace_back(region_.maxX(), i, front_[i].n);
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
  long long maxArea = region_.area();
  vector<int> candidates;

  long long area1 = 0;
  for (int t1 = starts.first; t1 < (int) sequences_.first.size(); ++t1) {
    Item item = sequences_.first[t1];
    area1 += item.area();
    if (area1 > maxArea) break;
    candidates.push_back(item.height);
    candidates.push_back(item.height + Params::minWaste);
    candidates.push_back(item.width);
    candidates.push_back(item.width  + Params::minWaste);
  }

  long long area2 = 0;
  for (int t2 = starts.second; t2 < (int) sequences_.second.size(); ++t2) {
    Item item = sequences_.second[t2];
    area2 += item.area();
    if (area2 > maxArea) break;
    candidates.push_back(item.height);
    candidates.push_back(item.height + Params::minWaste);
    candidates.push_back(item.width);
    candidates.push_back(item.width  + Params::minWaste);
  }

  candidates.push_back(region_.maxX());

  // TODO: make this smarter (multiple items)
  // TODO: take defects into account
  return candidates;
}

vector<pair<int, int> > PlateMerger::getParetoFront() const {
  return Merger::getParetoFront(region_.maxX());
}

PlateSolution PlateMerger::getSolution(pair<int, int> ends) {
  PlateSolution solution(region_);
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
    runCutMerger(prevElt.coord, curElt.coord, prevElt.n);
    CutSolution cut = cutMerger_.getSolution(curElt.n);
    solution.cuts.push_back(cut);
    cur = prev;
  }
  reverse(solution.cuts.begin(), solution.cuts.end());
  checkSolution(solution);
  return solution;
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

void PlateMerger::checkConsistency() const {
  Merger::checkConsistency();
  for (FrontElement elt : front_) {
    assert (isAdmissibleCutLine(elt.coord));
    assert ( (elt.coord == region_.minX()) == (elt.prev == -1));
  }
}
 

