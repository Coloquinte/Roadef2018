
#ifndef PACKER_HPP
#define PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"

class Packer {
 protected:
  Packer(const std::vector<Item> &sequence, const std::vector<Defect> &defects)
  : start_(0)
  , sequence_(sequence)
  , defects_(defects)
  , minWaste_(0)
  , minXX_(0)
  , maxXX_(0)
  , minYY_(0) {
  }

  int nItems() const {
    return sequence_.size();
  }

  bool fitsMinWaste(int a, int b) const {
      return a == b || a <= b - minWaste_;
  }

 protected:
  Rectangle region_;
  int start_;

  const std::vector<Item> &sequence_;
  const std::vector<Defect> &defects_;
  int minWaste_;
  int minXX_;
  int maxXX_;
  int minYY_;
};

#endif

