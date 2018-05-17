
#include "plate_packer.hpp"
#include "pareto_front.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

PlateSolution PlatePacker::run(const Problem &problem, int plateId, const vector<Item> &sequence, int start) {
  PlatePacker packer(problem, sequence);
  return packer.run(plateId, start);
}

int PlatePacker::count(const Problem &problem, int plateId, const vector<Item> &sequence, int start) {
  PlatePacker packer(problem, sequence);
  return packer.count(plateId, start);
}

PlatePacker::PlatePacker(const Problem &problem, const vector<Item> &sequence)
: Packer(problem, sequence)
, cutPacker_(problem, sequence)
, problem_(problem) {
}

PlateSolution PlatePacker::run(int plateId, int start) {
  Params p = problem_.params();
  Rectangle plate = Rectangle::FromCoordinates(0, 0, p.widthPlates, p.heightPlates);
  init(plate, start, problem_.plateDefects()[plateId]);

  // Dynamic programming on the first-level cuts
  assert (region_.minX() == 0);
  assert (region_.minY() == 0);

  front_.clear();
  front_.init(region_.minX(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    auto elt = front_[i];
    propagate(i, elt.valeur, elt.end);
  }
  front_.checkConsistency();

  return backtrack();
}

int PlatePacker::count(int plateId, int start) {
  return run(plateId, start).nItems();
}

int PlatePacker::countCut(int start, int minX, int maxX) {
  Rectangle cut = Rectangle::FromCoordinates(minX, region_.minY(), maxX, region_.maxY());
  return cutPacker_.count(cut, start, defects_);
}

CutSolution PlatePacker::packCut(int start, int minX, int maxX) {
  Rectangle cut = Rectangle::FromCoordinates(minX, region_.minY(), maxX, region_.maxY());
  return cutPacker_.run(cut, start, defects_);
}

vector<int> PlatePacker::computeBreakpoints() const {
  vector<int> ret;
  for (const Defect &defect : defects_) {
    ret.push_back(defect.maxX() + 1);
  }
  sort(ret.begin(), ret.end());
  return ret;
}


void PlatePacker::propagate(int previousFront, int previousItems, int beginCoord) {
  int maxEndCoord = min(region_.maxX(), beginCoord + maxXX_);
  for (int endCoord = maxEndCoord + minWaste_; endCoord >= beginCoord + minXX_; --endCoord) {
    CutSolution cutSolution = packCut(previousItems, beginCoord, endCoord);

    int maxUsed = cutSolution.maxUsedX();
    if (maxUsed + minWaste_ < endCoord) {
      // Shortcut from the current solution: no need to try all the next ones
      endCoord = maxUsed + minWaste_;
      cutSolution = packCut(previousItems, beginCoord, endCoord);
    }
    if (endCoord <= maxEndCoord)
      front_.insert(beginCoord, endCoord, previousItems + cutSolution.nItems(), previousFront);

    if (maxUsed < endCoord) {
      // Fully packed case
      cutSolution = packCut(previousItems, beginCoord, maxUsed);
      front_.insert(beginCoord, maxUsed, previousItems + cutSolution.nItems(), previousFront);
    }
  }
}

void PlatePacker::propagateBreakpoints(int after) {
  int from = front_[after].end;
  for (int bp : computeBreakpoints()) {
    if (bp <= from)
      continue;
    if (after + 1 < front_.size() && bp >= front_[after+1].end)
      break;
    // Find the previous front element we can extend
    int prev = 0;
    for (; prev < front_.size(); ++prev) {
      // Can we extend the previous row?
      if (front_[prev].end + minWaste_ > bp)
        break;
      // Can we create a row before?
      if (prev == 0 && front_[prev].end + minXX_ > bp)
        break;
    }
    --prev;
    if (prev < 0)
      continue;
    assert (prev <= after);

    // Propagate from here
    propagate(prev, front_[prev].valeur, bp);
  }
}

PlateSolution PlatePacker::backtrack() {
  vector<int> slices;
  slices.push_back(region_.maxX());

  int cur = front_.size() - 1;
  while (cur != 0) {
    auto elt = front_[cur];
    if (elt.begin + minXX_ > slices.back())
      continue;
    while (elt.begin + maxXX_ < slices.back())
      slices.push_back(slices.back() - maxXX_ + minXX_);

    // Keep residual on the last plate
    if (elt.valeur == nItems())
      slices.push_back(elt.end);
    slices.push_back(elt.begin);
    cur = elt.previous;
  }
  while (region_.minX() + maxXX_ < slices.back())
    slices.push_back(slices.back() - maxXX_ + minXX_);
  if (slices.back() != region_.minX())
    slices.push_back(region_.minX());
  reverse(slices.begin(), slices.end());

  int nPacked = start_;
  PlateSolution plateSolution(region_);
  for (size_t i = 0; i + 1 < slices.size(); ++i) {
    if (nPacked == nItems())
      break;
    CutSolution solution = packCut(nPacked, slices[i], slices[i+1]);
    nPacked += solution.nItems();
    assert (solution.width() >= minXX_);
    assert (solution.width() <= maxXX_);
    plateSolution.cuts.push_back(solution);
  }

  return plateSolution;
}

