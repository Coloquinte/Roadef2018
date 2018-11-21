
#ifndef PACKER_HPP
#define PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"

struct PlacementDescription {
  int maxUsedX;
  int maxUsedY;
  // Whether a minWaste gap is needed
  bool tightX;
  bool tightY;

  PlacementDescription() {
    maxUsedX = 0;
    maxUsedY = 0;
    tightX = true;
    tightY = true;
  }
};

class Packer {
 protected:
  Packer(const std::vector<Item> &sequence)
  : start_(0)
  , sequence_(sequence) {
  }

  Packer(const Problem &problem, const std::vector<Item> &sequence)
  : sequence_(sequence) {
    start_ = 0;
  }

  int nItems() const {
    return sequence_.size();
  }

  int nDefects() const {
    return defects_.size();
  }

  void init(Rectangle region, int start, const std::vector<Defect> &defects) {
    region_ = region;
    start_ = start;
    defects_.clear();
    for (Defect d : defects) {
      if (d.intersects(region_))
        defects_.push_back(d);
    }
  }

 protected:
  Rectangle region_;
  int start_;
  std::vector<Defect> defects_;

  const std::vector<Item> &sequence_;

  friend class RowPacker;
  friend class CutPacker;
  friend class PlatePacker;
};

#endif

