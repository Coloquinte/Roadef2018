
#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include "item.hpp"
#include "rectangle.hpp"

#include <vector>

struct ItemSolution {
  Item item;
  Rectangle pos;
};

struct RowSolution {
  Rectangle r;
  std::vector<ItemSolution> items;
};

struct CutSolution {
  Rectangle r;
  std::vector<RowSolution> rows;
};

struct PlateSolution {
  Rectangle r;
  std::vector<CutSolution> cuts;
};

struct Solution {
  std::vector<PlateSolution> plates;
};

#endif

