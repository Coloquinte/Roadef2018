
#include "packer_move.hpp"
#include "row_packer.hpp"
#include "cut_packer.hpp"
#include "plate_packer.hpp"
#include "sequence_packer.hpp"

#include <set>
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

namespace {

void findCandidatesBefore(vector<Item> &candidates, const vector<vector<Item> > &all, int subseqId, int maxArea) {
  set<int> stackSeen;
  for (int i = subseqId - 1; i >= 0; --i) {
    for (int j = all[i].size() - 1; j >= 0; --j) {
      Item item = all[i][j];
      if (stackSeen.count(item.stack))
        continue;
      stackSeen.insert(item.stack);
      if (item.area() > maxArea)
        continue;
      candidates.push_back(item);
    }
  }
}

void findCandidatesAfter(vector<Item> &candidates, const vector<vector<Item> > &all, int subseqId, int maxArea) {
  set<int> stackSeen;
  for (int i = subseqId + 1; i < (int) all.size(); ++i) {
    for (int j = 0; j < (int) all[i].size(); ++j) {
      Item item = all[i][j];
      if (stackSeen.count(item.stack))
        continue;
      stackSeen.insert(item.stack);
      if (item.area() > maxArea)
        continue;
      candidates.push_back(item);
    }
  }
}

}

void PackerMove::sequenceInsert(vector<Item> &sequence, mt19937 &rgen, const vector<vector<Item> > &all, int subseqId, int totalArea) {
  // Find the stacks in each sequence
  int maxArea = totalArea;
  for (Item item : sequence)  maxArea -= item.area();

  // TODO: prendre un item avant aussi bien qu'après
  vector<Item> candidates;
  findCandidatesBefore(candidates, all, subseqId, maxArea);
  findCandidatesAfter(candidates, all, subseqId, maxArea);
 
  if (candidates.empty()) return;

  int pickedPos = uniform_int_distribution<int>(0, candidates.size() - 1)(rgen);
  int insertionPos = uniform_int_distribution<int>(0, sequence.size())(rgen);
  sequence.insert(sequence.begin() + insertionPos, candidates[pickedPos]);
  repairSequence(sequence);
}

void PackerMove::sequenceShuffle(vector<Item> &sequence, mt19937 &rgen, const vector<vector<Item> > &all, int subseqId, int totalArea) {
  // Find the stacks in each sequence
  int maxArea = totalArea;
  for (Item item : sequence)  maxArea -= item.area();

  // TODO: prendre un item avant aussi bien qu'après
  vector<Item> candidates;
  findCandidatesBefore(candidates, all, subseqId, maxArea);
  findCandidatesAfter(candidates, all, subseqId, maxArea);
 
  if (candidates.empty()) return;

  int pickedPos = uniform_int_distribution<int>(0, candidates.size() - 1)(rgen);
  sequence.push_back(candidates[pickedPos]);
  shuffle(sequence.begin(), sequence.end(), rgen);
  repairSequence(sequence);
}

vector<Item> PackerMove::recreateFullSequence(const vector<Item> &newSubseq, const vector<vector<Item> > &all, int id) {
  set<int> seenIds;
  for (Item item : newSubseq) seenIds.insert(item.id);

  vector<Item> ret;
  for (int i = 0; i < id; ++i) {
    for (Item item : all[i]) {
      if (!seenIds.count(item.id))
        ret.push_back(item);
    }
  }

  for (Item item : newSubseq) {
    ret.push_back(item);
  }

  for (int i = id + 1; i < (int) all.size(); ++i) {
    for (Item item : all[i]) {
      if (!seenIds.count(item.id))
        ret.push_back(item);
    }
  }

  return ret;
}

Solution PackerMove::runPackRow(Rectangle targetRow, const std::vector<Item> &sequence, const std::vector<std::vector<Item> > &allRows, int rowId) {
  if (sequence.size() <= allRows[rowId].size()) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  RowPacker packer(sequence, params());
  RowSolution newSol = packer.run(targetRow, 0, problem().plateDefects()[plateIdOfRow(rowId)]);

  if (newSol.nItems() < (int) sequence.size()) {
    if (params().verbosity >= 4) {
      cout << "No improvement found" << endl;
    }
    return Solution();
  }
  else if (params().verbosity >= 4) {
    cout << "Improvement found" << endl;
  }

  // Run the whole algorithm with the new sequence if an improvement was found
  vector<Item> newSubseq = extractSequence(newSol);
  vector<Item> newSeq = recreateFullSequence(newSubseq, allRows, rowId);
  checkSequenceValid(newSeq);
  return runSequence(newSeq);
}

Solution PackerMove::runPackCut(Rectangle targetCut, const std::vector<Item> &sequence, const std::vector<std::vector<Item> > &allCuts, int cutId) {
  if (sequence.size() <= allCuts.size()) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  CutPacker packer(sequence, params());
  CutSolution newSol = packer.run(targetCut, 0, problem().plateDefects()[plateIdOfCut(cutId)]);

  if (newSol.nItems() < (int) sequence.size()) {
    if (params().verbosity >= 4) {
      cout << "No improvement found" << endl;
    }
    return Solution();
  }
  else if (params().verbosity >= 4) {
    cout << "Improvement found" << endl;
  }

  // Run the whole algorithm with the new sequence if an improvement was found
  vector<Item> newSubseq = extractSequence(newSol);
  vector<Item> newSeq = recreateFullSequence(newSubseq, allCuts, cutId);
  checkSequenceValid(newSeq);
  return runSequence(newSeq);
}

Solution PackerMove::runPackPlate(Rectangle targetPlate, const std::vector<Item> &sequence, const std::vector<std::vector<Item> > &allPlates, int plateId) {
  if (sequence.size() <= allPlates.size()) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  PlatePacker packer(problem(), sequence, params());
  PlateSolution newSol = packer.run(plateId, 0);

  if (newSol.nItems() < (int) sequence.size()) {
    if (params().verbosity >= 4) {
      cout << "No improvement found" << endl;
    }
    return Solution();
  }
  else if (params().verbosity >= 4) {
    cout << "Improvement found" << endl;
  }

  // Run the whole algorithm with the new sequence if an improvement was found
  vector<Item> newSubseq = extractSequence(newSol);
  vector<Item> newSeq = recreateFullSequence(newSubseq, allPlates, plateId);
  checkSequenceValid(newSeq);
  return runSequence(newSeq);

}

Solution PackRowInsert::apply(mt19937& rgen) {
  // Select a random row to reoptimize
  vector<RowSolution> rows = extractRows(solution());
  int rowId = uniform_int_distribution<int>(0, rows.size() - 1)(rgen);
  RowSolution targetRow = rows[rowId];

  auto allRows = extractRowItems(solution());
  vector<Item> sequence = extractSequence(rows[rowId]);
  sequenceInsert(sequence, rgen, allRows, rowId, targetRow.area());

  return runPackRow(targetRow, sequence, allRows, rowId);
}

Solution PackCutInsert::apply(mt19937& rgen) {
  // Select a random cut to reoptimize
  vector<CutSolution> cuts = extractCuts(solution());
  int cutId = uniform_int_distribution<int>(0, cuts.size() - 1)(rgen);
  CutSolution targetCut = cuts[cutId];

  auto allCuts = extractCutItems(solution());
  vector<Item> sequence = extractSequence(cuts[cutId]);
  sequenceInsert(sequence, rgen, allCuts, cutId, targetCut.area());

  return runPackCut(targetCut, sequence, allCuts, cutId);
}

Solution PackPlateInsert::apply(mt19937& rgen) {
  // Select a random plate to reoptimize
  vector<PlateSolution> plates = extractPlates(solution());
  int plateId = uniform_int_distribution<int>(0, plates.size() - 1)(rgen);
  PlateSolution targetPlate = plates[plateId];

  auto allPlates = extractPlateItems(solution());
  vector<Item> sequence = extractSequence(plates[plateId]);
  sequenceInsert(sequence, rgen, allPlates, plateId, targetPlate.area());

  return runPackPlate(targetPlate, sequence, allPlates, plateId);
}

Solution PackRowShuffle::apply(mt19937& rgen) {
  // Select a random row to reoptimize
  vector<RowSolution> rows = extractRows(solution());
  int rowId = uniform_int_distribution<int>(0, rows.size() - 1)(rgen);
  RowSolution targetRow = rows[rowId];

  auto allRows = extractRowItems(solution());
  vector<Item> sequence = extractSequence(rows[rowId]);
  sequenceShuffle(sequence, rgen, allRows, rowId, targetRow.area());

  return runPackRow(targetRow, sequence, allRows, rowId);
}

Solution PackCutShuffle::apply(mt19937& rgen) {
  // Select a random cut to reoptimize
  vector<CutSolution> cuts = extractCuts(solution());
  int cutId = uniform_int_distribution<int>(0, cuts.size() - 1)(rgen);
  CutSolution targetCut = cuts[cutId];

  auto allCuts = extractCutItems(solution());
  vector<Item> sequence = extractSequence(cuts[cutId]);
  sequenceShuffle(sequence, rgen, allCuts, cutId, targetCut.area());

  return runPackCut(targetCut, sequence, allCuts, cutId);
}

Solution PackPlateShuffle::apply(mt19937& rgen) {
  // Select a random plate to reoptimize
  vector<PlateSolution> plates = extractPlates(solution());
  int plateId = uniform_int_distribution<int>(0, plates.size() - 1)(rgen);
  PlateSolution targetPlate = plates[plateId];

  auto allPlates = extractPlateItems(solution());
  vector<Item> sequence = extractSequence(plates[plateId]);
  sequenceShuffle(sequence, rgen, allPlates, plateId, targetPlate.area());

  return runPackPlate(targetPlate, sequence, allPlates, plateId);
}

