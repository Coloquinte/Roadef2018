
#ifndef SOLUTION_HPP
#define SOLUTION_HPP

struct ItemSolution {
  Rectangle r;
  int item_id;
};

struct RowSolution {
  Rectangle r;
  std::vector<ItemSolution> items;
};

struct PlateSolution {
  Rectangle r;
  std::vector<RowSolution> rows;
};

struct Solution {
  std::vector<PlateSolution> plates;
};

#endif

