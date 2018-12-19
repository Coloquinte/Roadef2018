
#ifndef MERGER_MOVE_HPP
#define MERGER_MOVE_HPP

#include "move.hpp"

class MergerMove : public Move {
 protected:
  static std::pair<std::vector<Item>, std::vector<Item> > getMergeableSequences(std::mt19937 &rgen, const std::vector<Item> &subseq);
  static void extendMergeableSequences(std::pair<std::vector<Item>, std::vector<Item> > &sequences, std::mt19937 &rgen, const std::vector<Item> &all);
  static std::vector<std::pair<int, int> > getImprovingFront(const std::vector<std::pair<int, int> > &front, std::pair<int, int> best);
};

struct MergeRow : MergerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "MergeRow"; }
};

struct MergeCut : MergerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "MergeCut"; }
};

struct MergePlate : MergerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "MergePlate"; }
};

struct MergeAll : MergerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "MergeAll"; }
};


#endif

