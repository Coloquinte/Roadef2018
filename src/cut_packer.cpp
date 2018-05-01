
#include "cut_packer.hpp"
#include "pareto_front.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

CutSolution CutPacker::run(const Packer &parent, Rectangle cut, int start) {
  CutPacker packer(parent, cut, start);
  return packer.run();
}

int CutPacker::count(const Packer &parent, Rectangle cut, int start) {
  CutPacker packer(parent, cut, start);
  return packer.count();
}

CutPacker::CutPacker(const Packer &parent, Rectangle cut, int start)
: Packer(parent) {
  region_ = cut;
  start_ = start;
}

CutSolution CutPacker::run() {
  // Dynamic programming on the rows i.e. second-level cuts
  assert (region_.minY() == 0);
  int maxCoord = region_.maxY();

  ParetoFront front;
  front.insert(region_.minY(), start_, -1);
  for (int i = 0; i < front.size(); ++i) {
    auto elt = front[i];
    int beginCoord = elt.coord;
    int previousItems = elt.valeur;
    for (int endCoord = maxCoord; endCoord >= beginCoord + minYY_; --endCoord) {
      RowPacker::Quality result = countRow(previousItems, beginCoord, endCoord);

      // We cut all solutions with maxUsed + minWaste_ <= coord
      // Technically there may be non-dominated solutions within minWaste_ of the current one
      int maxUsed = result.maxUsedY;
      if (maxUsed + minWaste_ < endCoord) {
        // Shortcut from the current solution: no need to try all the next ones
        endCoord = maxUsed + minWaste_;
        result = countRow(previousItems, beginCoord, endCoord);
      }
      front.insert(endCoord, previousItems + result.nItems, i);

      if (maxUsed < endCoord) {
        // Try the fully packed case too
        result = countRow(previousItems, beginCoord, maxUsed);
        front.insert(maxUsed, previousItems + result.nItems, i);
      }
    }
  }
  front.checkConsistency();

  // Backtrack for the best solution
  CutSolution cutSolution(region_);
  int cur = front.size() - 1;
  bool lastRow = true;
  while (cur != 0) {
    auto eltEnd = front[cur];
    int next = eltEnd.previous;
    auto eltBegin = front[next];

    int beginCoord = eltBegin.coord;
    int endCoord = lastRow ? maxCoord : eltEnd.coord;

    auto solution = packRow(eltBegin.valeur, beginCoord, endCoord);
    // There is one corner case (minWaste_ at the end depending on orientation) where we catch a better solution with a smaller row
    assert (eltBegin.valeur + solution.nItems() == eltEnd.valeur || endCoord != eltEnd.coord);
    assert (solution.height() >= minYY_);
    cutSolution.rows.push_back(solution);
    cur = next;
    lastRow = false;
  }
  reverse(cutSolution.rows.begin(), cutSolution.rows.end());

  return cutSolution;
}

int CutPacker::count() {
  return run().nItems();
}

RowPacker::Quality CutPacker::countRow(int start, int minY, int maxY) const {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return RowPacker::count(*this, row, start);
}

RowSolution CutPacker::packRow(int start, int minY, int maxY) const {
  Rectangle row = Rectangle::FromCoordinates(region_.minX(), minY, region_.maxX(), maxY);
  return RowPacker::run(*this, row, start);
}

