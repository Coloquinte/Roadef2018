
#ifndef PACKER_MOVE_HPP
#define PACKER_MOVE_HPP

#include "move.hpp"

class PackerMove : public Move {
 protected:
  void sequenceInsert  (std::vector<Item> &sequence, std::mt19937 &rgen, const std::vector<std::vector<Item> > &all, int subseqId, int totalArea);
  void sequenceShuffle (std::vector<Item> &sequence, std::mt19937 &rgen, const std::vector<std::vector<Item> > &all, int subseqId, int totalArea);
  std::vector<Item> recreateFullSequence(const std::vector<Item> &newSubseq, const std::vector<std::vector<Item> > &all, int id);

  Solution runPackRow(Rectangle targetRow, const std::vector<Item> &sequence, const std::vector<std::vector<Item> > &allRows, int rowId);
  Solution runPackCut(Rectangle targetCut, const std::vector<Item> &sequence, const std::vector<std::vector<Item> > &allCuts, int cutId);
  Solution runPackPlate(Rectangle targetPlate, const std::vector<Item> &sequence, const std::vector<std::vector<Item> > &allPlates, int plateId);
};

struct PackRowInsert : PackerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "PackRowInsert"; }
};

struct PackCutInsert : PackerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "PackCutInsert"; }
};

struct PackPlateInsert : PackerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "PackPlateInsert"; }
};

struct PackRowShuffle : PackerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "PackRowShuffle"; }
};

struct PackCutShuffle : PackerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "PackCutShuffle"; }
};

struct PackPlateShuffle : PackerMove {
  virtual Solution apply(std::mt19937& rgen);
  virtual std::string name() const { return "PackPlateShuffle"; }
};

#endif

