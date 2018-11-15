
#include "plate_packer.hpp"
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
  Rectangle plate = Rectangle::FromCoordinates(0, 0, Params::widthPlates, Params::heightPlates);
  init(plate, start, problem_.plateDefects()[plateId]);
  sort(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxX() < b.maxX();
        });
  assert (region_.minX() == 0);
  assert (region_.minY() == 0);

  front_.clear();
  front_.init(region_.minX(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    auto elt = front_[i];
    propagate(i, elt.valeur, elt.end);
    propagateBreakpoints(i);
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

void PlatePacker::propagate(int previousFront, int previousItems, int beginCoord) {
  int maxEndCoord = min(region_.maxX(), beginCoord + Params::maxXX);
  for (int endCoord = maxEndCoord + Params::minWaste; endCoord >= beginCoord + Params::minXX; --endCoord) {
    CutSolution cutSolution = packCut(previousItems, beginCoord, endCoord);

    int maxUsed = cutSolution.maxUsedX();
    // All solutions with "maxUsed + Params::minWaste <= end" are considered dominated
    if (maxUsed + Params::minWaste < endCoord) {
      // Solve the packed case, which might contain more items in cornercases
      CutSolution packed = packCut(previousItems, beginCoord, maxUsed + Params::minWaste);
      if (packed.nItems() >= cutSolution.nItems()) {
        cutSolution = packed;
        endCoord = maxUsed + Params::minWaste;
      }
    }
    if (endCoord <= maxEndCoord)
      front_.insert(beginCoord, endCoord, previousItems + cutSolution.nItems(), previousFront);

    if (maxUsed < endCoord) {
      // Try the tight case too, else the first minWaste iterations could short-circuit it
      // TODO: handle this case in another manner (for example start after maxX)
      cutSolution = packCut(previousItems, beginCoord, maxUsed);
      front_.insert(beginCoord, maxUsed, previousItems + cutSolution.nItems(), previousFront);
    }
  }
}

void PlatePacker::propagateBreakpoints(int after) {
  int from = front_[after].end;
  assert (is_sorted(defects_.begin(), defects_.end(),
        [](const Defect &a, const Defect &b) {
          return a.maxX() < b.maxX();
        }));
  for (const Defect &defect : defects_) {
    int bp = defect.maxX() + 1;
    if (bp <= from)
      continue;
    if (after + 1 < front_.size() && bp >= front_[after+1].end)
      continue;
    // Find the previous front element we can extend
    int prev = 0;
    for (; prev < front_.size(); ++prev) {
      // Can we extend the previous row?
      // TODO: a row may already include some waste
      if (front_[prev].end + Params::minWaste > bp)
        break;
      // Can we create a row before?
      if (prev == 0 && front_[prev].end + Params::minXX > bp)
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
    if (elt.valeur == nItems()) {
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

  return plateSolution;
}

bool PlatePacker::isAdmissibleCutLine(int x) const {
  for (Defect d : defects_) {
    if (d.intersectsVerticalLine(x))
      return false;
  }
  return true;
}

