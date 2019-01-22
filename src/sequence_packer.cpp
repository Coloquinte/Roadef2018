// Copyright (C) 2019 Gabriel Gouvine - All Rights Reserved

#include "sequence_packer.hpp"
#include "plate_packer.hpp"

#include <cassert>
#include <algorithm>

/*
 * Dynamic programming on all cutting points
 */

using namespace std;

Solution SequencePacker::run(const Problem &problem, const vector<Item> &sequence, SolverParams options, const Solution &existing) {
  SequencePacker packer(problem, sequence, options, existing);
  packer.run();
  return packer.solution_;
}

SequencePacker::SequencePacker(const Problem &problem, const vector<Item> &sequence, SolverParams options, const Solution &existing)
: problem_(problem)
, existingSolution_(existing)
, sequence_(sequence)
, options_(options) {
  packedItems_ = 0;
  packedExistingItems_ = 0;
}

int SequencePacker::sequenceBeginDiff() const {
  vector<Item> existingSeq = existingSolution_.sequence(problem_);
  int beginDiff = 0;
  for (beginDiff = 0; beginDiff < (int) sequence_.size(); ++beginDiff) {
    int ind = beginDiff;
    if (ind < (int) existingSeq.size()
     && existingSeq[ind].id != sequence_[ind].id) break;
  }
  return beginDiff;
}

int SequencePacker::sequenceEndDiff() const {
  vector<Item> existingSeq = existingSolution_.sequence(problem_);
  int endDiff = sequence_.size();
  for (; endDiff > 0; --endDiff) {
    int ind = endDiff - 1;
    if (ind < (int) existingSeq.size()
     && existingSeq[ind].id != sequence_[ind].id) break;
  }
  if (endDiff == 0) endDiff = sequence_.size();
  return endDiff;
}

void SequencePacker::runNoCancel() {
  while (solution_.nPlates() < Params::nPlates) {
    if (packedItems_ == (int) sequence_.size()) break;
    PlateSolution plate = PlatePacker::run(problem_, sequence_, options_, solution_.nPlates(), packedItems_);
    packedItems_ += plate.nItems();
    solution_.plates.push_back(plate);
  }
}

void SequencePacker::runEarlyCancel() {
  int beginDiff = sequenceBeginDiff();
  int endDiff = sequenceEndDiff();

  while (solution_.nPlates() < Params::nPlates) {
    if (packedItems_ == (int) sequence_.size()) {
      break;
    }

    if (packedExistingItems_ > endDiff) {
      // Worse solution: early break
      if (packedExistingItems_ > packedItems_) {
        solution_ = Solution();
        break;
      }
      // Reuse the end of the previous solution
      if (packedExistingItems_ == packedItems_) {
        while (solution_.nPlates() < existingSolution_.nPlates()) {
          solution_.plates.push_back(existingSolution_.plates[solution_.nPlates()]);
        }
        break;
      }
      // Good: let's go on
    }

    PlateSolution existingPlate;
    if (solution_.nPlates() < existingSolution_.nPlates())
      existingPlate = existingSolution_.plates[solution_.nPlates()];
    packedExistingItems_ += existingPlate.nItems();

    PlateSolution plate;
    if (solution_.nPlates() < existingSolution_.nPlates() && packedExistingItems_ < beginDiff) {
      // Reuse the beginning of the previous solution
      // Strict to allow one more item to be inserted
      plate = existingPlate;
    }
    else {
      plate = PlatePacker::run(problem_, sequence_, options_, solution_.nPlates(), packedItems_);
    }
    packedItems_ += plate.nItems();
    solution_.plates.push_back(plate);
  }
}

void SequencePacker::run() {
  if (options_.earlyCancel)
    runEarlyCancel();
  else
    runNoCancel();
}

