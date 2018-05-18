
#include "ordering_heuristic.hpp"

#include <algorithm>

using namespace std;

vector<Item> OrderingHeuristic::orderShuffle(const Problem &problem, mt19937 &rgen, int chunkSize) {
  OrderingHeuristic heuristic(problem, rgen);
  heuristic.orderShuffle(chunkSize);
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

void OrderingHeuristic::orderShuffle(int chunkSize) {
  shuffle(leftover_.begin(), leftover_.end(), rgen_);
  while (!leftover_.empty()) {
    uniform_int_distribution<size_t> stackDist(0, leftover_.size() - 1);
    size_t stackInd = stackDist(rgen_);

    int maxPopped = min(chunkSize, (int) leftover_[stackInd].size());
    uniform_int_distribution<int> poppedDist(1, maxPopped);
    int popped = poppedDist(rgen_);

    for (int i = 0; i < popped; ++i) {
      ordering_.push_back(leftover_[stackInd].back());
      leftover_[stackInd].pop_back();
    }
    if (leftover_[stackInd].empty())
      leftover_.erase(leftover_.begin() + stackInd);
  }
  leftover_.clear();
}


