
#ifndef SEQUENCE_PACKER_HPP
#define SEQUENCE_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"

class SequencePacker {
 public:
  static Solution run(const Problem &problem, const std::vector<Item> &sequence);
  static Solution run(const Problem &problem, const std::vector<int> &sequence);

 private:
  SequencePacker(const Problem &problem, const std::vector<Item> &sequence);
  void run();

  int nItems() const { return sequence_.size(); }

 private:
  const Problem &problem_;
  const std::vector<Item> &sequence_;

  Solution solution_;
  int packedItems_;
};

#endif

