// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef ORDERING_HEURISTICS_HPP
#define ORDERING_HEURISTICS_HPP

#include "problem.hpp"

#include <random>

class OrderingHeuristic {
 public:
  static std::vector<Item> orderShuffle(const Problem &problem, std::mt19937 &rgen, int chunkSize, int wasteThreshold=1);
  static std::vector<Item> orderShuffle(const Problem &problem, std::mt19937 &rgen,
      const std::vector<Item> &initial, int chunkSize, int windowSize, int wasteThreshold=1);

 private:
  OrderingHeuristic(const Problem &problem, std::mt19937 &rgen, int wasteThreshold);
  void init();
  void init(const std::vector<Item> &sequence, int begin, int end);
  void orderShuffle(int chunkSize);

  void takeFirstStackElement(int stackInd);
  void takeFromRandomStack();
  void takeFromBestStack();
  int wasteEstimate(int h1, int w1, int h2, int w2) const;
  int wasteEstimate(const Item& item1, const Item& item2) const;

 private:
  std::vector<Item> before_;
  std::vector<Item> after_;
  std::vector<std::vector<Item> > leftover_;

  std::vector<Item> ordering_;

  const Problem &problem_;
  std::mt19937 &rgen_;
  int wasteThreshold_;
  std::size_t nLeftover_;
};

#endif

