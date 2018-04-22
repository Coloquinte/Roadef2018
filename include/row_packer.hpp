
#ifndef ROW_PACKER_HPP
#define ROW_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"

class RowPacker {
 public:
  struct Quality {
    int nItems;
    int maxX;
  };

 public:
  // + pass defects
  static RowSolution run(Rectangle row, const std::vector<Item> &sequence, int start, int minWaste);
  static Quality count(Rectangle row, const std::vector<Item> &sequence, int start, int minWaste);

 private:
  RowPacker(Rectangle row, const std::vector<Item> &sequence, int start, int minWaste);
  RowSolution run();
  Quality count();

  int nItems() const {
    return sequence_.size();
  }
  bool fitsMinWaste(int a, int b) const {
      return a == b || a <= b - minWaste_;
  }

 private:
  const Rectangle row_;
  const std::vector<Item> &sequence_;
  const int start_;
  const int minWaste_;
};

#endif

