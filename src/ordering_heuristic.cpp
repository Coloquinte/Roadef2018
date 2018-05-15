
#include "ordering_heuristic.hpp"

#include <algorithm>

using namespace std;

vector<Item> OrderingHeuristic::orderShuffleStacks(const Problem &problem, mt19937 &rgen) {
  OrderingHeuristic heuristic(problem, rgen);
  heuristic.orderShuffleStacks();
  return heuristic.ordering_;
}

vector<Item> OrderingHeuristic::orderShuffle(const Problem &problem, mt19937 &rgen) {
  OrderingHeuristic heuristic(problem, rgen);
  heuristic.orderShuffle();
  return heuristic.ordering_;
}

vector<Item> OrderingHeuristic::orderSizeHeuristic(const Problem &problem, mt19937 &rgen) {
  OrderingHeuristic heuristic(problem, rgen);
  heuristic.orderSizeHeuristic();
  return heuristic.ordering_;
}

OrderingHeuristic::OrderingHeuristic(const Problem &problem, mt19937 &rgen)
: leftover_(problem.stackItems())
, rgen_(rgen) {
  for (vector<Item> &left : leftover_) {
    reverse(left.begin(), left.end());
  }
  nLeftover_ = 0;
  for (const vector<Item> &left : leftover_) {
    nLeftover_ += left.size();
  }
}

void OrderingHeuristic::orderShuffleStacks() {
  shuffle(leftover_.begin(), leftover_.end(), rgen_);
  for (const vector<Item> &left : leftover_) {
    ordering_.insert(ordering_.end(), left.rbegin(), left.rend());
  }
  leftover_.clear();
}

void OrderingHeuristic::orderShuffle() {
  for (; nLeftover_ > 0; --nLeftover_) {
    takeRandomItem();
  }
}

void OrderingHeuristic::orderSizeHeuristic() {
  if (nLeftover_ == 0)
    return;
  takeRandomItem();
  --nLeftover_;
  for (; nLeftover_ > 0; --nLeftover_) {
    shuffle(leftover_.begin(), leftover_.end(), rgen_);
    int targetWidth = ordering_.back().width;
    int bestDiff = -1;
    size_t bestInd = 0;
    // Pick the element that has the closest size to the latest element
    for (size_t i = 0; i < leftover_.size(); ++i) {
      if (leftover_[i].empty()) continue;
      int diff = abs(leftover_[i].back().width - targetWidth);
      if (bestDiff == -1 || diff < bestDiff) {
        bestDiff = diff;
        bestInd = i;
      }
    }
    ordering_.push_back(leftover_[bestInd].back());
    leftover_[bestInd].pop_back();
  }
}

void OrderingHeuristic::takeRandomItem() {
  uniform_int_distribution<size_t> dist(0, nLeftover_ - 1);
  size_t eltInd = dist(rgen_);
  size_t stackInd = 0;
  while (eltInd >= leftover_[stackInd].size()) {
    eltInd -= leftover_[stackInd].size();
    ++stackInd;
  }
  ordering_.push_back(leftover_[stackInd].back());
  leftover_[stackInd].pop_back();
}


