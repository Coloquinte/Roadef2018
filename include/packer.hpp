
#ifndef PACKER_HPP
#define PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"

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

  int firstValidVerticalCut(int minX, bool tightX) const;
  int firstValidHorizontalCut(int minY, bool tightY) const;
  int firstValidVerticalCutFrom(int fromX, int minX, bool tightX) const;
  int firstValidHorizontalCutFrom(int fromY, int minY, bool tightY) const;

  void checkConsistency() const;
  void checkItems() const;
  void checkDefects() const;

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

