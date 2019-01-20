// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef PACKER_HPP
#define PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "solver_params.hpp"

class Packer {
 protected:
  Packer(const std::vector<Item> &sequence, SolverParams options)
  : start_(0)
  , sequence_(sequence)
  , options_(options) {
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

  static std::vector<int> extractFrontChanges(const std::vector<int> &front);

 protected:
  Rectangle region_;
  int start_;
  std::vector<Defect> defects_;

  const std::vector<Item> &sequence_;
  SolverParams options_;

  friend class RowPacker;
  friend class CutPacker;
  friend class PlatePacker;
};

#endif

