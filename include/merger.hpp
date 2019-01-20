// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

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
    int prev;
    std::pair<int, int> n;

    FrontElement(int coord, int prev, std::pair<int, int> n)
    : coord(coord)
    , prev(prev)
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

  static void keepOnlyNonDominated(std::vector<std::pair<int, int> > &front);
  static void checkRefines(const std::vector<std::pair<int, int> > &pareto, const std::vector<std::pair<int, int> > &optimistic);

  void eraseDominated(int coord, int nFirst, int nSecond, int distance);
  bool isDominated(int coord, int nFirst, int nSecond, int distance);

  std::vector<std::pair<int, int> > getParetoFront(int coord) const;
  void insertFront(int coord, int prev, int nFirst, int nSecond);
  void insertFrontCleanup(int coord, int prev, int nFirst, int nSecond, int distance=0);

  void checkConsistency() const;

 protected:
  Rectangle region_;
  std::vector<Defect> defects_;

  const std::pair<std::vector<Item>, std::vector<Item> > &sequences_;
  SolverParams options_;

  std::vector<FrontElement> front_;
  std::vector<std::pair<int, int> > starts_;

  friend class RowMerger;
  friend class CutMerger;
  friend class PlateMerger;
};

#endif

