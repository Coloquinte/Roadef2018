
#include "cut_packer.hpp"
#include "plate_packer.hpp"
#include "pareto_front.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

PlateSolution PlatePacker::run(const Problem &problem, int plateId, const std::vector<Item> &sequence, int start) {
  PlatePacker packer(problem, plateId, sequence, start);
  return packer.run();
}

int PlatePacker::count(const Problem &problem, int plateId, const std::vector<Item> &sequence, int start) {
  PlatePacker packer(problem, plateId, sequence, start);
  return packer.count();
}

PlatePacker::PlatePacker(const Problem &problem, int plateId, const std::vector<Item> &sequence, int start)
: Packer(sequence, problem.plateDefects()[plateId]) {
  Params p = problem.params();
  start_ = start;
  minXX_ = p.minXX;
  maxXX_ = p.maxXX;
  minYY_ = p.minYY;
  minWaste_ = p.minWaste;
  pitchX_ = 50;
  pitchY_ = 50;
  region_ = Rectangle::FromCoordinates(0, 0, p.widthPlates, p.heightPlates);
}

PlateSolution PlatePacker::run() {
  // Dynamic programming on the first-level cuts
  assert (region_.minX() == 0);
  assert (region_.minY() == 0);

  int maxCoord = region_.maxX();

  ParetoFront front;
  front.insert(region_.minX(), start_, -1);
  for (int i = 0; i < front.size(); ++i) {
    auto elt = front[i];
    int beginCoord = elt.coord;
    int previousItems = elt.valeur;
    for (int endCoord = min(maxCoord, beginCoord + maxXX_); endCoord >= beginCoord + minXX_; --endCoord) {
      Rectangle cut = Rectangle::FromCoordinates(beginCoord, region_.minY(), endCoord, region_.maxY());
      CutSolution cutSolution = CutPacker::run(*this, cut, previousItems);

      int maxUsed = cutSolution.maxUsedX();
      if (maxUsed + minWaste_ < endCoord) {
        // Shortcut from the current solution: no need to try all the next ones
        endCoord = maxUsed + minWaste_;
        cut = Rectangle::FromCoordinates(beginCoord, region_.minY(), endCoord, region_.maxY());
        cutSolution = CutPacker::run(*this, cut, previousItems);
      }
      front.insert(endCoord, previousItems + cutSolution.nItems(), i);

      if (maxUsed < endCoord) {
        // Fully packed case
        cut = Rectangle::FromCoordinates(beginCoord, region_.minY(), maxUsed, region_.maxY());
        cutSolution = CutPacker::run(*this, cut, previousItems);
        front.insert(maxUsed, previousItems + cutSolution.nItems(), i);
      }
    }
  }
  front.checkConsistency();

  // Backtrack for the best solution
  // FIXME: get to the first element of the Pareto front that fits
  PlateSolution plateSolution(region_);
  int cur = front.size() - 1;
  while (cur != 0) {
    auto eltEnd = front[cur];
    int next = eltEnd.previous;
    auto eltBegin = front[next];

    Rectangle cut = Rectangle::FromCoordinates(eltBegin.coord, region_.minY(), eltEnd.coord, region_.maxY());
    auto solution = CutPacker::run(*this, cut, eltBegin.valeur);
    assert (eltBegin.valeur + solution.nItems() == eltEnd.valeur);
    plateSolution.cuts.push_back(solution);
    cur = next;
  }
  reverse(plateSolution.cuts.begin(), plateSolution.cuts.end());

  return plateSolution;
}

int PlatePacker::count() {
  return run().nItems();
}

