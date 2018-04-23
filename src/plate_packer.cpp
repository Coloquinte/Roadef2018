
#include "cut_packer.hpp"
#include "plate_packer.hpp"
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
  int maxIndex = divRoundUp(maxCoord, pitchX_);
  int minSpacing = divRoundUp(minXX_, pitchX_);
  int maxSpacing = divRoundDown(maxXX_, pitchX_);

  std::vector<int> packingVec(maxIndex + 1, start_);
  std::vector<int> previousVec(maxIndex + 1, 0);

  for (int end = minSpacing; end <= maxIndex; ++end) {
    int bestPacking = start_;
    int bestPrevious = max(0, end - maxSpacing);

    for (int begin = max(0, end - maxSpacing); begin <= end - minSpacing; ++begin) {
      int beginCoord = begin * pitchX_;
      int endCoord = min(end * pitchX_, maxCoord);
      Rectangle cut = Rectangle::FromCoordinates(beginCoord, 0, endCoord, region_.maxY());
      int previousItems = packingVec[begin];
      int cutCount = CutPacker::count(*this, cut, previousItems).nItems;
      int packing = previousItems + cutCount;

      if (packing > bestPacking) {
        bestPacking = packing;
        bestPrevious = begin;
      }
    }

    packingVec[end] = bestPacking;
    previousVec[end] = bestPrevious;
  }

  // Backtrack for the best solution
  PlateSolution plateSolution(region_);
  int cur = maxIndex;
  // For the last plate, cut as soon as possible
  while (cur != 0 && packingVec[cur-1] == (int) sequence_.size())
    --cur;
  while (cur != 0) {
    int end = cur;
    int begin = previousVec[end];
    int beginCoord = begin * pitchX_;
    int endCoord = min(end * pitchX_, maxCoord);

    Rectangle cut = Rectangle::FromCoordinates(beginCoord, 0, endCoord, region_.maxY());
    auto solution = CutPacker::run(*this, cut, packingVec[begin]);
    assert (packingVec[begin] + solution.nItems() == packingVec[end]);
    plateSolution.cuts.push_back(solution);
    cur = begin;
  }
  reverse(plateSolution.cuts.begin(), plateSolution.cuts.end());

  return plateSolution;
}

int PlatePacker::count() {
  return run().nItems();
}

