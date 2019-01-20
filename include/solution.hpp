// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include "item.hpp"
#include "node.hpp"
#include "rectangle.hpp"

#include <vector>
#include <iosfwd>

struct ItemSolution : Rectangle {
  ItemSolution() {}
  ItemSolution(Rectangle r, int itemId)
    : Rectangle(r)
    , itemId(itemId) {
    }
  void report() const;

  int itemId;
};

struct RowSolution : Rectangle {
  RowSolution() {}
  RowSolution(Rectangle r)
    : Rectangle(r) {
      items.reserve(4);
    }
  void report() const;
  std::vector<int> sequence() const;

  std::vector<ItemSolution> items;

  int nItems() const;
  int maxUsedY() const;
  int maxUsedX() const;
};

struct CutSolution : Rectangle {
  CutSolution() {}
  CutSolution(Rectangle r)
    : Rectangle(r) {
      rows.reserve(4);
    }
  void report() const;
  std::vector<int> sequence() const;

  std::vector<RowSolution> rows;

  int nItems() const;
  int nRows() const { return rows.size(); }
  int maxUsedX() const;
};

struct PlateSolution : Rectangle {
  PlateSolution() {}
  PlateSolution(Rectangle r)
    : Rectangle(r) {
      cuts.reserve(4);
    }
  void report() const;
  std::vector<int> sequence() const;

  std::vector<CutSolution> cuts;

  int nItems() const;
  int nCuts() const { return cuts.size(); }
};

struct Solution {
  std::vector<PlateSolution> plates;

  int nItems() const;
  int nPlates() const { return plates.size(); }

  void report() const;
  std::vector<int> sequence() const;
  void write(std::string fileName) const;

  static std::vector<int> readOrdering(std::string filename);
};

#endif

