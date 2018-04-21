
#ifndef SRDERING_HEURISTICS_HPP
#define ORDERING_HEURISTICS_HPP

#include "problem.hpp"

#include <random>

class OrderingHeuristic {
 public:
  static std::vector<Item> orderKeepStacks(const Problem &problem, unsigned seed);
  static std::vector<Item> orderShuffleStacks(const Problem &problem, unsigned seed);

 private:
  OrderingHeuristic(const Problem &problem, unsigned seed);

  std::vector<Item> orderKeepStacks();
  std::vector<Item> orderShuffleStacks();
  void takeRandomSequence();
  void takeRandomItem();

 private:
  std::mt19937 rgen_;
  std::vector<std::vector<Item> > leftover_;
  std::vector<Item> ordering_;
};

#endif

