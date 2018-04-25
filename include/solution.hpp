
#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include "item.hpp"
#include "rectangle.hpp"

#include <vector>
#include <iosfwd>

struct ItemSolution : Rectangle {
  ItemSolution() {}
  ItemSolution(Rectangle r) : Rectangle(r) {}
  ItemSolution(int x, int y, int w, int h);

  int itemId;
};

struct RowSolution : Rectangle {
  RowSolution() {}
  RowSolution(Rectangle r) : Rectangle(r) {}
  RowSolution(int x, int y, int w, int h);

  std::vector<ItemSolution> items;

  int nItems() const;
  int maxUsedY() const;
  int maxUsedX() const;
};

struct CutSolution : Rectangle {
  CutSolution() {}
  CutSolution(Rectangle r) : Rectangle(r) {}
  CutSolution(int x, int y, int w, int h);

  std::vector<RowSolution> rows;

  int nItems() const;
  int maxUsedX() const;
};

struct PlateSolution : Rectangle {
  PlateSolution() {}
  PlateSolution(Rectangle r) : Rectangle(r) {}
  PlateSolution(int x, int y, int w, int h);

  std::vector<CutSolution> cuts;

  int nItems() const;
};

struct Solution {
  std::vector<PlateSolution> plates;

  int nItems() const;
  void write(std::ostream &) const;
};

#endif

