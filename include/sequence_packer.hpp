
#ifndef SEQUENCE_PACKER_HPP
#define SEQUENCE_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "solver_params.hpp"

class SequencePacker {
 public:
  static Solution run(const Problem &problem, const std::vector<Item> &sequence, SolverParams options);

 private:
  SequencePacker(const Problem &problem, const std::vector<Item> &sequence, SolverParams options);
  void run();

  int nItems() const { return sequence_.size(); }

 private:
  const Problem &problem_;
  const std::vector<Item> &sequence_;
  SolverParams options_;

  Solution solution_;
  int packedItems_;
};

#endif

