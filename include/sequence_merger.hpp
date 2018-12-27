
#ifndef SEQUENCE_MERGER_HPP
#define SEQUENCE_MERGER_HPP

#include "problem.hpp"
#include "solution.hpp"
#include "plate_merger.hpp"

class SequenceMerger {
 public:
  static Solution run(const Problem &problem, const std::pair<std::vector<Item>, std::vector<Item> > &sequences, SolverParams options);

 private:
  SequenceMerger(const Problem &problem, const std::pair<std::vector<Item>, std::vector<Item> > &sequences, SolverParams options);
  Solution run();

  void checkConsistency() const;

 private:
  void runPlateMerger(int plateId, std::pair<int, int> starts);

 private:
  const Problem &problem_;
  const std::pair<std::vector<Item>, std::vector<Item> > &sequences_;
  SolverParams options_;
};

#endif

