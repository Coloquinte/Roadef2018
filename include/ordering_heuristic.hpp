
#ifndef ORDERING_HEURISTICS_HPP
#define ORDERING_HEURISTICS_HPP

#include "problem.hpp"

#include <random>

class OrderingHeuristic {
 public:
  static std::vector<Item> orderShuffleStacks(const Problem &problem, std::mt19937 &rgen);
  static std::vector<Item> orderShuffle(const Problem &problem, std::mt19937 &rgen);
  static std::vector<Item> orderSizeHeuristic(const Problem &problem, std::mt19937 &rgen);

 private:
  OrderingHeuristic(const Problem &problem, std::mt19937 &rgen);

  void orderShuffleStacks();
  void orderShuffle();
  void orderSizeHeuristic();

  void takeRandomItem();

 private:
  std::vector<std::vector<Item> > leftover_;
  std::vector<Item> ordering_;
  std::mt19937 &rgen_;
  std::size_t nLeftover_;
};

#endif

