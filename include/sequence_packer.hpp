
#ifndef SEQUENCE_PACKER_HPP
#define SEQUENCE_PACKER_HPP

#include "problem.hpp"
#include "solution.hpp"

class SequencePacker {
 public:
  static Solution run(const Problem &problem, const std::vector<Item> &sequence, int pitchX=1, int pitchY=1);

 private:
  SequencePacker(const Problem &problem, const std::vector<Item> &sequence, int pitchX, int pitchY);
  void run();

  PlateSolution packPlate(int fromItem, Rectangle plate);
  CutSolution packCut(int fromItem, Rectangle cut);
  RowSolution packRow(int fromItem, Rectangle row);
  int countPackCut(int fromItem, Rectangle cut);
  int countPackRow(int fromItem, Rectangle row);

  // TODO: handle defects
  int nItems() const { return sequence_.size(); }

  bool fitsMinWaste(int a, int b) const;

 private:
  const Problem &problem_;
  const std::vector<Item> &sequence_;

  Solution solution_;
  int packedItems_;

  int pitchX_;
  int pitchY_;
};

#endif

