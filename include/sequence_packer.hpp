// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#ifndef SEQUENCE_PACKER_HPP
#define SEQUENCE_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "solver_params.hpp"

class SequencePacker {
 public:
  static Solution run(const Problem &problem, const std::vector<Item> &sequence, SolverParams options, const Solution &existing=Solution());

 private:
  SequencePacker(const Problem &problem, const std::vector<Item> &sequence, SolverParams options, const Solution &existing);
  void run();
  void runNoCancel();
  void runEarlyCancel();

  int nItems() const { return sequence_.size(); }
  int sequenceBeginDiff() const;
  int sequenceEndDiff() const;

 private:
  const Problem &problem_;
  const Solution &existingSolution_;
  const std::vector<Item> &sequence_;
  SolverParams options_;

  Solution solution_;
  int packedItems_;
  int packedExistingItems_;
};

#endif

