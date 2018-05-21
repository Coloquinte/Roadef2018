
#include "ordering_heuristic.hpp"

#include <algorithm>
#include <unordered_set>

using namespace std;

vector<Item> OrderingHeuristic::orderShuffle(const Problem &problem, mt19937 &rgen, int chunkSize) {
  OrderingHeuristic heuristic(problem, rgen);
  heuristic.init();
  heuristic.orderShuffle(chunkSize);
  return heuristic.ordering_;
}

vector<Item> OrderingHeuristic::orderShuffle(const Problem &problem, mt19937 &rgen,
    const std::vector<Item> &initial, int chunkSize, int windowSize) {
  OrderingHeuristic heuristic(problem, rgen);
  windowSize = min(windowSize, (int) initial.size());
  uniform_int_distribution<int> beginDist(0, initial.size() - windowSize);
  int windowBegin = beginDist(rgen);
  heuristic.init(initial, windowBegin, windowBegin + windowSize);
  heuristic.orderShuffle(chunkSize);
  return heuristic.ordering_;
}

OrderingHeuristic::OrderingHeuristic(const Problem &problem, mt19937 &rgen)
: problem_(problem)
, rgen_(rgen) {
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
  before_.clear();
  after_.clear();
}


