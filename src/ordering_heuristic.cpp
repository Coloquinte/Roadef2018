// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#include "ordering_heuristic.hpp"

#include <algorithm>
#include <unordered_set>
#include <cassert>

using namespace std;

vector<Item> OrderingHeuristic::orderShuffle(const Problem &problem, mt19937 &rgen, int chunkSize, int wasteThreshold) {
  OrderingHeuristic heuristic(problem, rgen, wasteThreshold);
  heuristic.init();
  heuristic.orderShuffle(chunkSize);
  return heuristic.ordering_;
}

vector<Item> OrderingHeuristic::orderShuffle(const Problem &problem, mt19937 &rgen,
    const std::vector<Item> &initial, int chunkSize, int maxWindowSize, int wasteThreshold) {
  assert (maxWindowSize >= 3);
  uniform_int_distribution<int> windowDist(3, maxWindowSize);
  int windowSize = min(windowDist(rgen), (int) initial.size());

  OrderingHeuristic heuristic(problem, rgen, wasteThreshold);
  uniform_int_distribution<int> beginDist(0, initial.size() - windowSize);
  int windowBegin = beginDist(rgen);

  heuristic.init(initial, windowBegin, windowBegin + windowSize);
  heuristic.orderShuffle(chunkSize);
  return heuristic.ordering_;
}

OrderingHeuristic::OrderingHeuristic(const Problem &problem, mt19937 &rgen, int wasteThreshold)
: problem_(problem)
, rgen_(rgen)
, wasteThreshold_(wasteThreshold) {
}

void OrderingHeuristic::init() {
  unordered_set<int> fixed;
  for (const Item &item : before_)
    fixed.insert(item.id);
  for (const Item &item : after_)
    fixed.insert(item.id);

  for (const vector<Item> &pbStack : problem_.stackItems()) {
    vector<Item> stack;
    for (const Item &item : pbStack) {
      if (fixed.count(item.id) == 0)
        stack.push_back(item);
    }
    reverse(stack.begin(), stack.end());
    if (!stack.empty())
      leftover_.push_back(stack);
  }

  nLeftover_ = 0;
  for (const vector<Item> &left : leftover_) {
    nLeftover_ += left.size();
  }
}

void OrderingHeuristic::init(const std::vector<Item> &sequence, int begin, int end) {
  before_.insert(before_.end(), sequence.begin(), sequence.begin() + begin);
  after_.insert(after_.end(), sequence.begin() + end, sequence.end());
  init();
}

int OrderingHeuristic::wasteEstimate(int h1, int w1, int h2, int w2) const {
  if (h1 == h2) {
    return 0;
  }
  else if (h1 > h2) {
    int waste = max(h1 - h2, wasteThreshold_);
    return w2 * waste;
  }
  else {
    int waste = max(h2 - h1, wasteThreshold_);
    return w1 * waste;
  }
}

int OrderingHeuristic::wasteEstimate(const Item& item1, const Item& item2) const {
  // Estimates how much waste putting those two elements together will generate
  int wasteVV = wasteEstimate(item1.height, item1.width, item2.height, item2.width);
  int wasteHV = wasteEstimate(item1.width, item1.height, item2.height, item2.width);
  int wasteVH = wasteEstimate(item1.height, item1.width, item2.width, item2.height);
  int wasteHH = wasteEstimate(item1.width, item1.height, item2.width, item2.height);
  return min({wasteVV, wasteHV, wasteVH, wasteHH});
}

void OrderingHeuristic::takeFirstStackElement(int stackInd) {
  assert (!leftover_[stackInd].empty());
  ordering_.push_back(leftover_[stackInd].back());
  leftover_[stackInd].pop_back();
  --nLeftover_;

  // Remove the stack if empty
  if (leftover_[stackInd].empty())
    leftover_.erase(leftover_.begin() + stackInd);
}

void OrderingHeuristic::takeFromRandomStack() {
  uniform_int_distribution<size_t> stackDist(0, leftover_.size() - 1);
  takeFirstStackElement(stackDist(rgen_));
}

void OrderingHeuristic::takeFromBestStack() {
  assert (!ordering_.empty());
  assert (!leftover_.empty());
  Item lastItem = ordering_.back();
  std::vector<int> candidates;
  int bestWaste = -1;
  for (int i = 0; i < (int) leftover_.size(); ++i) {
    assert (!leftover_[i].empty());
    int curWaste = wasteEstimate(lastItem, leftover_[i].back());
    if (candidates.empty() || curWaste < bestWaste) {
      candidates.clear();
      candidates.push_back(i);
      bestWaste = curWaste;
    }
    else if (curWaste == bestWaste) {
      candidates.push_back(i);
    }
  }
  assert (!candidates.empty());
  uniform_int_distribution<int> selDist(0, candidates.size() - 1);
  takeFirstStackElement(candidates[selDist(rgen_)]);
}

void OrderingHeuristic::orderShuffle(int chunkSize) {
  // Consume all the fixed items before
  ordering_.insert(ordering_.end(), before_.begin(), before_.end());

  while (nLeftover_ > 0) {
    // Decide how many items will be added in this run
    uniform_int_distribution<int> poppedDist(1, min(chunkSize, (int) nLeftover_));
    int nPopped = poppedDist(rgen_);

    takeFromRandomStack();
    for (int i = 1; i < nPopped; ++i) {
      takeFromBestStack();
    }
  }

  // Consume all the fixed items after
  ordering_.insert(ordering_.end(), after_.begin(), after_.end());

  // Cleanup
  leftover_.clear();
  before_.clear();
  after_.clear();
}


