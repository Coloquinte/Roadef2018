// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef MERGER_MOVE_HPP
#define MERGER_MOVE_HPP

#include "move.hpp"

class MergerMove : public Move {
 protected:
  static std::pair<std::vector<Item>, std::vector<Item> > getRandomStacksSequences(std::mt19937 &rgen, const std::vector<Item> &subseq);
  static std::pair<std::vector<Item>, std::vector<Item> > getOneStackSequences(std::mt19937 &rgen, const std::vector<Item> &subseq);

  static void extendMergeableSequences(std::pair<std::vector<Item>, std::vector<Item> > &sequences, std::mt19937 &rgen, const std::vector<std::vector<Item> > &all, int subseqId, int totalArea);
  static std::vector<std::pair<int, int> > getImprovingFront(const std::vector<std::pair<int, int> > &front, std::pair<int, int> best);
  static std::vector<Item> recreateFullSequence(const std::vector<Item> &newSubseq, const std::vector<std::vector<Item> > &all, int id);
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

struct MergeRandomStacks : MergerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "MergeRandomStacks"; }
};

struct MergeOneStack : MergerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "MergeOneStack"; }
};

#endif

