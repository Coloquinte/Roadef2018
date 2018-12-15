
#include "row_merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;


RowMerger::RowMerger(SolverParams options, const std::pair<std::vector<Item>, std::vector<Item> > &sequences)
: Merger(options, sequences) {
}

void RowMerger::init(Rectangle row, const std::vector<Defect> &defects, std::pair<int, int> starts) {
  std::vector<std::pair<int, int> > vec;
  vec.push_back(starts);
  init(row, defects, vec);
}

void RowMerger::init(Rectangle row, const std::vector<Defect> &defects, const std::vector<std::pair<int, int> > &starts) {
  Merger::init(row, defects, starts, row.minX());
}

void RowMerger::buildFront() {
  if (options_.rowMerging == PackingOption::Approximate) {
    buildFrontApproximate();
  }
  else {
    buildFrontExact();
  }
}

void RowMerger::buildFrontApproximate() {

}

void RowMerger::buildFrontExact() {

}

std::vector<std::pair<int, int> > RowMerger::getParetoFront() const {
  std::vector<std::pair<int, int> > paretoFront;
  for (auto elt : front_) {
    if (elt.coord == region_.maxX())
      paretoFront.push_back(elt.end);
  }
  // TODO: filter dominated elements out
  return paretoFront;
}

RowSolution RowMerger::getSolution(std::pair<int, int> ends) const {
  RowSolution solution(region_);
  // TODO
  return solution;
}

bool RowMerger::canPlaceDown(int x, int width, int height) const {
  Rectangle place = Rectangle::FromCoordinates(x, region_.minY(), x + width, region_.minY() + height);
  for (Defect d : defects_) {
    if (d.intersects(place))
      return false;
  }
  return true;
}

bool RowMerger::canPlaceUp(int x, int width, int height) const {
  Rectangle place = Rectangle::FromCoordinates(x, region_.maxY() - height, x + width, region_.maxY());
  for (Defect d : defects_) {
    if (d.intersects(place))
      return false;
  }
  return true;
}

bool RowMerger::isAdmissibleCutLine(int x) const {
  if (x == region_.minX() || x == region_.maxX())
    return true;
  if (x < region_.minX() + Params::minWaste || x > region_.maxX() - Params::minWaste)
    return false;
  for (Defect d : defects_) {
    if (d.intersectsVerticalLine(x))
      return false;
  }
  return true;
}

