
#include "plate_packer.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

using namespace std;

PlateSolution PlatePacker::run(const Problem &problem, const vector<Item> &sequence, SolverParams options, int plateId, int start) {
  PlatePacker packer(problem, sequence, options);
  return packer.run(plateId, start);
}

PlatePacker::PlatePacker(const Problem &problem, const vector<Item> &sequence, SolverParams options)
: Packer(sequence, options)
, cutPacker_(sequence, options)
, problem_(problem) {
}

PlateSolution PlatePacker::runExact() {
  // Fill the front
  std::vector<int> front(Params::widthPlates + 1, -1);
  std::vector<int> prev(Params::widthPlates + 1, -1);
  front[0] = start_;
  for (int j = Params::minXX; j <= Params::widthPlates; ++j) {
    int best = -1;
    int pred = -1;
    if (!isAdmissibleCutLine(j)) continue;
    for (int i = max(0, j - Params::maxXX); i <= j - Params::minXX; ++i) {
      if (front[i] < 0) continue;
      int cnt = front[i] + countCut(front[i], i, j).nItems;
      if (cnt > best) {
        best = cnt;
        pred = i;
      }
      front[j] = best;
      prev[j] = pred;
    }
  }

  // Special case where we can finish earlier for the last plate
  int finishLine = Params::widthPlates;
  for (int j = Params::minXX; j <= Params::widthPlates; ++j) {
    if (front[j] == nItems()) {
      finishLine = j;
      break;
    }
  }

  // Build the slices
  int cur = finishLine;
  slices_.clear();
  slices_.push_back(finishLine);
  while (cur != 0) {
    cur = prev[cur];
    slices_.push_back(cur);
  }
  reverse(slices_.begin(), slices_.end());

  return backtrack();
}

PlateSolution PlatePacker::runApproximate() {
  // Fill the front
  front_.clear();
  front_.init(region_.minX(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    propagate(i, front_[i].end);
    propagateBreakpoints(i);
  }
  front_.checkConsistency();

  // Build the slices
  slices_.clear();
  slices_.push_back(region_.maxX());

  int cur = front_.size() - 1;
  while (cur != 0) {
    auto elt = front_[cur];
    if (elt.begin + Params::minXX > slices_.back())
      continue;
    while (elt.begin + Params::maxXX < slices_.back())
      slices_.push_back(slices_.back() - Params::maxXX + Params::minXX);

    // Keep residual on the last plate
    if (elt.value == nItems()) {
      slices_.clear();
      slices_.push_back(elt.end);
    }
    assert (elt.previous < cur);
    assert (elt.begin < slices_.back());
    slices_.push_back(elt.begin);
    cur = elt.previous;
  }
  while (region_.minX() + Params::maxXX < slices_.back())
    slices_.push_back(slices_.back() - Params::maxXX + Params::minXX);
  if (slices_.back() != region_.minX())
    slices_.push_back(region_.minX());
  reverse(slices_.begin(), slices_.end());

  return backtrack();
}

PlateSolution PlatePacker::runDiagnostic() {
  PlateSolution approximate = runApproximate();
  PlateSolution exact = runExact();
  if (approximate.nItems() != exact.nItems()) {
    cout << "Exact plate algorithm obtains " << exact.nItems() << " items but approximate one obtains " << approximate.nItems() << endl;
    cout << "Exact" << endl;
    exact.report();
    cout << "Approximate" << endl;
    approximate.report();
    cout << endl;
  }
  else if (approximate.nCuts() != 0 && exact.nCuts() != 0 && approximate.cuts.back().maxX() != exact.cuts.back().maxX()) {
    cout << "Exact plate algorithm ends plate at " << exact.cuts.back().maxX() << " but approximate one obtains " << approximate.cuts.back().maxX() << endl;
    cout << "Exact" << endl;
    exact.report();
    cout << "Approximate" << endl;
    approximate.report();
    cout << endl;
  }
  return exact;
}

void PlatePacker::setup(int plateId, int start) {
  Rectangle plate = Rectangle::FromCoordinates(0, 0, Params::widthPlates, Params::heightPlates);
  init(plate, start, problem_.plateDefects()[plateId]);
  sort(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxX() < b.maxX();
        });
  assert (region_.minX() == 0);
  assert (region_.minY() == 0);
  assert (region_.maxX() == Params::widthPlates);
  assert (region_.maxY() == Params::heightPlates);
}

PlateSolution PlatePacker::run(int plateId, int start) {
  setup(plateId, start);
  if (options_.platePacking == PackingOption::Approximate) {
    return runApproximate();
  }
  else if (options_.platePacking == PackingOption::Exact) {
    return runExact();
  }
  else {
    return runDiagnostic();
  }
}

CutPacker::CutDescription PlatePacker::countCut(int start, int minX, int maxX) {
  Rectangle cut = Rectangle::FromCoordinates(minX, region_.minY(), maxX, region_.maxY());
  return cutPacker_.count(cut, start, defects_);
}

CutSolution PlatePacker::packCut(int start, int minX, int maxX) {
  Rectangle cut = Rectangle::FromCoordinates(minX, region_.minY(), maxX, region_.maxY());
  return cutPacker_.run(cut, start, defects_);
}

void PlatePacker::propagate(int previousFront, int beginCoord) {
  int previousItems = front_[previousFront].value;
  int maxEndCoord = min(region_.maxX(), beginCoord + Params::maxXX);
  for (int endCoord = maxEndCoord + Params::minWaste; endCoord >= beginCoord + Params::minXX; --endCoord) {
    if (!isAdmissibleCutLine(endCoord)) continue;
    CutPacker::CutDescription result = countCut(previousItems, beginCoord, endCoord);
    if (result.nItems == 0) break;
    if (result.maxUsedX <= maxEndCoord && utils::fitsMinWaste(result.maxUsedX, result.tightX, region_.maxX()))
      front_.insert(beginCoord, result.maxUsedX, previousItems + result.nItems, previousFront);
    // One more attempt, but tight this time
    if (!result.tightX) {
      CutPacker::CutDescription tight = countCut(previousItems, beginCoord, result.maxUsedX - Params::minWaste);
      if (tight.maxUsedX <= maxEndCoord && utils::fitsMinWaste(tight.maxUsedX, tight.tightX, region_.maxX()))
        front_.insert(beginCoord, tight.maxUsedX, previousItems + tight.nItems, previousFront);
    }
    endCoord = min(endCoord, result.tightX ? result.maxUsedX + Params::minWaste : result.maxUsedX);
  }
}

void PlatePacker::propagateBreakpoints(int after) {
  int from = front_[after].end;
  int to = after + 1 < front_.size() ? front_[after+1].end : region_.maxX();
  assert (is_sorted(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxX() < b.maxX();
        }));
  for (const Defect &defect : defects_) {
    int bp = defect.maxX() + 1;
    // Only consider defects that are in between the current front elements
    if (bp <= from)
      continue;
    if (bp >= to)
      continue;
    // Find the previous front elements we can extend, by cutting as early as possible after the defect
    //    * minWaste after the maxUsedX for a cut
    //    * minXX after the start of the region
    //    * right after the defect
    // Note that whenever a cut is not admissible, another defect will account for a valid one (as long as it's not tight)
    for (int i = 1; i <= after; ++i) {
      int cutPos = front_[i].end + Params::minWaste;
      if (cutPos > bp && isAdmissibleCutLine(cutPos))
        propagate(i, cutPos);
    }
    int firstCutPos = region_.minX() + Params::minXX;
    if (bp < firstCutPos && isAdmissibleCutLine(firstCutPos)) {
      propagate(0, firstCutPos);
    }
    int maxValid = 0;
    for (int i = 1; i <= after; ++i) {
      if (front_[i].end + Params::minWaste <= bp)
        maxValid = i;
    }
    if (isAdmissibleCutLine(bp) && bp >= region_.minX() + Params::minXX) {
      propagate(maxValid, bp);
    }
  }
}

PlateSolution PlatePacker::backtrack() {
  int nPacked = start_;
  PlateSolution plateSolution(region_);
  for (size_t i = 0; i + 1 < slices_.size(); ++i) {
    if (nPacked == nItems())
      break;
    CutSolution solution = packCut(nPacked, slices_[i], slices_[i+1]);
    nPacked += solution.nItems();
    assert (solution.width() >= Params::minXX);
    assert (solution.width() <= Params::maxXX);
    plateSolution.cuts.push_back(solution);
  }
  assert (slices_.back() <= region_.maxX());
  assert (nPacked == nItems() || slices_.back() == region_.maxX());

  return plateSolution;
}

bool PlatePacker::isAdmissibleCutLine(int x) const {
  if (x == 0 || x == Params::widthPlates)
    return true;
  if (x < Params::minXX)
    return false;
  for (Defect d : defects_) {
    if (d.intersectsVerticalLine(x))
      return false;
  }
  return true;
}

