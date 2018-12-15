
#ifndef MERGER_HPP
#define MERGER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "solver_params.hpp"

#include <utility>

class Merger {
 public:
  struct FrontElement {
    int coord;
    int pred;
    std::pair<int, int> n;

    FrontElement(int coord, int pred, std::pair<int, int> n)
    : coord(coord)
    , pred(pred)
    , n(n) {
    }
  };

 protected:
  Merger(SolverParams options, const std::pair<std::vector<Item>, std::vector<Item> > &sequences)
  : sequences_(sequences)
  , options_(options) {
  }

  int nDefects() const {
    return defects_.size();
  }

  void init(Rectangle region, const std::vector<Defect> &defects, const std::vector<std::pair<int, int> > &starts, int coord);

  void checkConsistency() const;

 protected:
  Rectangle region_;
  std::vector<Defect> defects_;

  const std::pair<std::vector<Item>, std::vector<Item> > &sequences_;
  SolverParams options_;

  std::vector<FrontElement> front_;

  friend class RowMerger;
  friend class CutMerger;
  friend class PlateMerger;
};

#endif

