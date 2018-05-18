
#ifndef ORDERING_HEURISTICS_HPP
#define ORDERING_HEURISTICS_HPP

#include "problem.hpp"

#include <random>

class OrderingHeuristic {
 public:
  static std::vector<Item> orderShuffle(const Problem &problem, std::mt19937 &rgen, int chunkSize);

 private:
  OrderingHeuristic(const Problem &problem, std::mt19937 &rgen);
  void orderShuffle(int chunkSize);

 private:
  std::vector<std::vector<Item> > leftover_;
  std::vector<Item> ordering_;
  std::mt19937 &rgen_;
  std::size_t nLeftover_;
};

#endif

