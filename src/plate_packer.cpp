
#include "plate_packer.hpp"
#include "pareto_front.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

PlateSolution PlatePacker::run(const Problem &problem, int plateId, const vector<Item> &sequence, int start) {
  PlatePacker packer(problem, plateId, sequence, start);
  return packer.run();
}

int PlatePacker::count(const Problem &problem, int plateId, const vector<Item> &sequence, int start) {
  PlatePacker packer(problem, plateId, sequence, start);
  return packer.count();
}

PlatePacker::PlatePacker(const Problem &problem, int plateId, const vector<Item> &sequence, int start)
: Packer(sequence, problem.plateDefects()[plateId]) {
  Params p = problem.params();
  start_ = start;
  minXX_ = p.minXX;
  maxXX_ = p.maxXX;
  minYY_ = p.minYY;
  minWaste_ = p.minWaste;
  region_ = Rectangle::FromCoordinates(0, 0, p.widthPlates, p.heightPlates);
}

PlateSolution PlatePacker::run() {
  // Dynamic programming on the first-level cuts
  assert (region_.minX() == 0);
  assert (region_.minY() == 0);

  front_.init(region_.minX(), start_);
  for (int i = 0; i < front_.size(); ++i) {
    auto elt = front_[i];
    propagate(i, elt.valeur, elt.end);
  }
  front_.checkConsistency();

  return backtrack();
}

int PlatePacker::count() {
  return run().nItems();
}

int PlatePacker::countCut(int start, int minX, int maxX) const {
  Rectangle cut = Rectangle::FromCoordinates(minX, region_.minY(), maxX, region_.maxY());
  return CutPacker::count(*this, cut, start);
}

CutSolution PlatePacker::packCut(int start, int minX, int maxX) const {
  Rectangle cut = Rectangle::FromCoordinates(minX, region_.minY(), maxX, region_.maxY());
  return CutPacker::run(*this, cut, start);
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

PlateSolution PlatePacker::backtrack() {
  PlateSolution plateSolution(region_);

  int maxCoord = region_.maxX();
  int cur = front_.size() - 1;
  bool lastCut = true;

  while (cur != 0) {
    auto elt = front_[cur];
    int next = elt.previous;
    auto prevElt = front_[next];

    int beginCoord = elt.begin;
    int endCoord = elt.end;
    if (lastCut && elt.valeur != nItems()) {
      // End but not the residual
      if (maxCoord - beginCoord <= maxXX_) {
        // Extend the last cut to the full plate
        endCoord = maxCoord;
      }
      else if (maxCoord - endCoord >= minXX_) {
        // Not possible to extend, add another cut instead
        // FIXME: when we need to add several new cuts; not possible with standard dimensions
        Rectangle emptyCut = Rectangle::FromCoordinates(endCoord, region_.minY(), maxCoord, region_.maxY());
        plateSolution.cuts.emplace_back(emptyCut);
      }
      else {
        // Not possible to add a cut; try again
        --cur;
        continue;
      }
    }

    auto solution = packCut(prevElt.valeur, beginCoord, endCoord);
    assert (prevElt.valeur + solution.nItems() == elt.valeur || endCoord != elt.end);
    assert (solution.width() >= minXX_);
    assert (solution.width() <= maxXX_);
    plateSolution.cuts.push_back(solution);
    cur = next;
    lastCut = false;
  }
  reverse(plateSolution.cuts.begin(), plateSolution.cuts.end());

  return plateSolution;

}

