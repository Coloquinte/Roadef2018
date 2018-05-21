
#ifndef ORDERING_HEURISTICS_HPP
#define ORDERING_HEURISTICS_HPP

#include "problem.hpp"

#include <random>

class OrderingHeuristic {
 public:
  static std::vector<Item> orderShuffle(const Problem &problem, std::mt19937 &rgen, int chunkSize);
  static std::vector<Item> orderShuffle(const Problem &problem, std::mt19937 &rgen,
      const std::vector<Item> &initial, int chunkSize, int windowSize);

 private:
  OrderingHeuristic(const Problem &problem, std::mt19937 &rgen);
  void init();
  void init(const std::vector<Item> &sequence, int begin, int end);
  void orderShuffle(int chunkSize);

 private:
  std::vector<Item> before_;
  std::vector<Item> after_;
  std::vector<std::vector<Item> > leftover_;

  std::vector<Item> ordering_;

  const Problem &problem_;
  std::mt19937 &rgen_;
  std::size_t nLeftover_;
};

#endif

