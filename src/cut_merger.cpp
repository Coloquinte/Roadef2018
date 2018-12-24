
#include "cut_merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

CutMerger::CutMerger(SolverParams options, const pair<vector<Item>, vector<Item> > &sequences)
: Merger(options, sequences)
, rowMerger_(options, sequences) {
}

void CutMerger::init(Rectangle cut, const vector<Defect> &defects, pair<int, int> starts) {
  vector<pair<int, int> > vec;
  vec.push_back(starts);
  init(cut, defects, vec);
}

void CutMerger::init(Rectangle cut, const vector<Defect> &defects, const vector<pair<int, int> > &starts) {
  Merger::init(cut, defects, starts, cut.minX());
}

void CutMerger::buildFront() {
  if (options_.cutMerging == PackingOption::Approximate) {
    buildFrontApproximate();
  }
  else {
    buildFrontExact();
  }
}

void CutMerger::buildFrontApproximate() {
  for (int i = 0; i < (int) front_.size(); ++i) {
    // Propagate from front element i
    FrontElement elt = front_[i];
    vector<int> candidates = getMaxYCandidates(elt.coord, elt.n);
    for (int endCoord : candidates) {
      runRowMerger(elt.coord, endCoord, elt.n);
      for (pair<int, int> n : rowMerger_.getParetoFront()) {
        insertFrontCleanup(endCoord, i, n.first, n.second);
      }
    }
    // TODO: propagate from defects
  }
}

void CutMerger::buildFrontExact() {
  // TODO
}

void CutMerger::runRowMerger(int minY, int maxY, pair<int, int> starts) {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  rowMerger_.init(row, defects_, starts);
  rowMerger_.buildFront();
}

vector<int> CutMerger::getMaxYCandidates(int minY, pair<int, int> starts) {
  vector<int> candidates;
  for (int t1 = starts.first; t1 < (int) sequences_.first.size(); ++t1) {
    Item item = sequences_.first[t1];
    candidates.push_back(item.width);
    candidates.push_back(item.height);
    candidates.push_back(item.width  + Params::minWaste);
    candidates.push_back(item.height + Params::minWaste);
  }
  for (int t2 = starts.second; t2 < (int) sequences_.second.size(); ++t2) {
    Item item = sequences_.second[t2];
    candidates.push_back(item.width);
    candidates.push_back(item.height);
    candidates.push_back(item.width  + Params::minWaste);
    candidates.push_back(item.height + Params::minWaste);
  }
  // TODO: break on area limit
  return candidates;
}

vector<pair<int, int> > CutMerger::getParetoFront() const {
  vector<pair<int, int> > paretoFront;
  for (FrontElement elt : front_) {
    if (elt.coord == region_.maxX())
      paretoFront.push_back(elt.n);
  }
  // TODO: filter dominated elements out
  return paretoFront;
}

CutSolution CutMerger::getSolution(pair<int, int> ends) {
  CutSolution solution(region_);
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
    runRowMerger(prevElt.coord, curElt.coord, prevElt.n);
    RowSolution row = rowMerger_.getSolution(curElt.n);
    solution.rows.push_back(row);
    cur = prev;
  }
  reverse(solution.rows.begin(), solution.rows.end());
  assert (solution.nItems() == ends.first + ends.second);
  checkSolution(solution);
  return solution;
}

void CutMerger::checkSolution(const CutSolution &cut) const {
  if (cut.rows.empty()) return;

  for (const RowSolution &row: cut.rows) {
    assert (cut.contains(row));
    assert (row.maxX() == cut.maxX() && row.minX() == cut.minX());
    assert (isAdmissibleCutLine(row.minY()));
    assert (isAdmissibleCutLine(row.maxY()));
  }

  assert (cut.minY() == cut.rows.front().minY());
  assert (cut.rows.back().maxY() == cut.maxY());

  for (int i = 0; i+1 < (int) cut.rows.size(); ++i) {
    RowSolution row1 = cut.rows[i];
    RowSolution row2 = cut.rows[i+1];
    assert (row1.maxY() == row2.minY());
  }
}

bool CutMerger::isAdmissibleCutLine(int y) const {
  if (y == region_.minY() || y == region_.maxY())
    return true;
  if (y < region_.minY() + Params::minYY || y > region_.maxY() - Params::minYY)
    return false;
  for (Defect d : defects_) {
    if (d.intersectsHorizontalLine(y))
      return false;
  }
  return true;
}

void CutMerger::checkConsistency() const {
  Merger::checkConsistency();
  for (FrontElement elt : front_) {
    assert (isAdmissibleCutLine(elt.coord));
    assert ( (elt.coord == region_.minY()) == (elt.prev == -1));
  }
}
 

