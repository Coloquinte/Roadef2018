
#include "cut_merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>
#include <queue>

using namespace std;

CutMerger::CutMerger(SolverParams options, const pair<vector<Item>, vector<Item> > &sequences)
: Merger(options, sequences)
, rowMerger_(options, sequences)
, nCalls_(0) {
}

void CutMerger::init(Rectangle cut, const vector<Defect> &defects, pair<int, int> starts) {
  vector<pair<int, int> > vec;
  vec.push_back(starts);
  init(cut, defects, vec);
}

void CutMerger::init(Rectangle cut, const vector<Defect> &defects, const vector<pair<int, int> > &starts) {
  Merger::init(cut, defects, starts, cut.minY());
}

void CutMerger::buildFront() {
  ++nCalls_;
  if (options_.cutMerging == PackingOption::Approximate) {
    buildFrontApproximate();
  }
  else {
    buildFrontExact();
  }
}

void CutMerger::buildFrontApproximate() {
  for (int i = 0; i < (int) front_.size(); ++i) {
    propagateFromElement(i);
    // TODO: propagate from and to defects
  }
  propagateFrontToEnd();
  checkConsistency();
}

void CutMerger::propagateFromElement(int i) {
  FrontElement elt = front_[i];
  priority_queue<int> candidates;
  const int startCoord = region_.maxY();
  candidates.push(startCoord);
  int maxSeenCoord = startCoord + 1;
  while (!candidates.empty()) {
    int coord = candidates.top();
    candidates.pop();
    if (coord >= maxSeenCoord) continue;
    if (coord < elt.coord + Params::minYY) continue;
    maxSeenCoord = coord;
    if (isRowDominated(elt.coord, coord, elt.n)) {
      continue;
    }
    runRowMerger(elt.coord, coord, elt.n);
    for (pair<int, int> n : rowMerger_.getParetoFront()) {
      if (isAdmissibleCutLine(coord))
        insertFrontCleanup(coord, i, n.first, n.second, Params::minWaste);
      RowSolution row = rowMerger_.getSolution(n);
      int exactCandidate = getMaxUsedY(row);
      // TODO: if the cut is tight, try exactCandidate + Params::minWaste - 1
      assert (isAdmissibleCutLine(exactCandidate));
      candidates.push(exactCandidate);
      int previousCandidate = exactCandidate - 1;
      makeAdmissible(previousCandidate);
      candidates.push(previousCandidate);
    }
  }

  /*
  vector<int> candidates = getMaxYCandidates(elt.coord, elt.n);
  for (int endCoord : candidates) {
    if (endCoord > region_.maxY())
      return;
    if (endCoord - elt.coord < Params::minYY)
      return;
    if (!isAdmissibleCutLine(endCoord))
      return;
    // TODO: quick filtering without calling the lower level
    runRowMerger(elt.coord, endCoord, elt.n);
    for (pair<int, int> n : rowMerger_.getParetoFront()) {
      insertFrontCleanup(endCoord, i, n.first, n.second, Params::minWaste);
    }
  }
  */
}

void CutMerger::propagateFrontToEnd() {
  int origFrontSize = front_.size();
  for (int i = 0; i < origFrontSize; ++i) {
    if (front_[i].coord > region_.maxY() - Params::minYY)
      continue;
    front_.emplace_back(region_.maxY(), i, front_[i].n);
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

bool CutMerger::isEndDominated(int coord, pair<int, int> n) const {
  for (FrontElement elt : front_) {
    if (elt.coord + Params::minWaste > coord) continue;
    if (elt.n.first >= n.first && elt.n.second >= n.second)
      return true;
  }
  return false;
}

bool CutMerger::isRowDominated(int minY, int maxY, pair<int, int> starts) {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  rowMerger_.init(row, defects_, starts);
  vector<pair<int, int> > optimisticFront = rowMerger_.optimisticParetoFront();
  for (pair<int, int> n : optimisticFront) {
    if (!isEndDominated(maxY, n))
      return false;
  }
  return true;
}

void CutMerger::addMaxYCandidates(vector<int> &candidates, int minY, const vector<Item> &sequence, int start) {
  int minTotWidth = 0;
  int minDim = 0;
  for (int t = start; t < (int) sequence.size(); ++t) {
    Item item = sequence[t];
    minTotWidth += item.width;
    minDim = max(minDim, item.width);
    if (minTotWidth > region_.width()) break;
    if (item.height >= minDim)
      candidates.push_back(minY + item.height);
    if (item.height + Params::minWaste >= minDim)
      candidates.push_back(minY + item.height + Params::minWaste);
    if (item.width >= minDim)
      candidates.push_back(minY + item.width);
    if (item.width + Params::minWaste >= minDim)
      candidates.push_back(minY + item.width  + Params::minWaste);
  }
}

vector<int> CutMerger::getMaxYCandidates(int minY, pair<int, int> starts) {
  // TODO: better estimation of the maximum number of items that can be packed
  vector<int> candidates;

  addMaxYCandidates(candidates, minY, sequences_.first, starts.first);
  addMaxYCandidates(candidates, minY, sequences_.second, starts.second);

  candidates.push_back(region_.maxY());

  sort(candidates.begin(), candidates.end());
  candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end());

  // TODO: take defects into account
  return candidates;
}

void CutMerger::makeAdmissible(int &y) const {
  while (!isAdmissibleCutLine(y))
    --y;
}

int CutMerger::getMaxUsedY(const RowSolution &row) const {
  int maxY = row.minY();
  for (const ItemSolution &item : row.items) {
    maxY = max(maxY, item.maxY());
  }
  bool fits = true;
  for (const ItemSolution &item : row.items) {
    fits &= utils::fitsMinWaste(item.maxY(), maxY);
  }
  int maxUsedY = maxY;
  if (!fits || maxY < row.minY() + Params::minYY || !isAdmissibleCutLine(maxY)) {
    maxUsedY = max(maxY + Params::minWaste, row.minY() + Params::minYY);
  }
  makeAdmissible(maxUsedY);
  assert (maxUsedY <= region_.maxY());
  return maxUsedY;
}

vector<pair<int, int> > CutMerger::getParetoFront() const {
  vector<pair<int, int> > pareto = Merger::getParetoFront(region_.maxY());
  checkRefines(pareto, optimisticParetoFront());
  return pareto;
}

vector<int> CutMerger::getUsableItemAreas(const vector<Item> &sequence, int start) const {
  vector<int> areas;
  int maxArea = region_.area();
  int totArea = 0;
  for (int i = start; i < (int) sequence.size(); ++i) {
    Item item = sequence[i];
    if(!utils::fitsMinWaste(item.width, region_.height())
    && !utils::fitsMinWaste(item.width, region_.width()))
      break;

    totArea += item.area();
    if (totArea > maxArea)
      break;
    areas.push_back(item.area());
  }
  return areas;
}

vector<pair<int, int> > CutMerger::optimisticParetoFront() const {
  vector<pair<int, int> > ret;
  int maxArea = region_.area();
  for (pair<int, int> start : starts_) {
    pair<vector<int>, vector<int> > areas;
    areas.first = getUsableItemAreas(sequences_.first, start.first);
    areas.second = getUsableItemAreas(sequences_.second, start.second);

    pair<int, int> totArea;
    totArea.first = 0;
    totArea.second = accumulate(areas.second.begin(), areas.second.end(), 0);
    size_t j = areas.second.size();
    for (size_t i = 0; i <= areas.first.size(); ++i) {
      if (i > 0)
        totArea.first += areas.first[i-1];
      for (; totArea.first + totArea.second > maxArea && j > 0; --j) {
        totArea.second -= areas.second[j-1];
      }
      ret.emplace_back( start.first + (int) i, start.second + (int) j );
    }
  }
  keepOnlyNonDominated(ret);
  return ret;
}

CutSolution CutMerger::getSolution(pair<int, int> ends) {
  CutSolution solution(region_);
  // Find the corresponding element on the front
  int cur = -1;
  for (int i = 0; i < (int) front_.size(); ++i) {
    FrontElement elt = front_[i];
    if (elt.coord == region_.maxY() && elt.n == ends)
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
 

