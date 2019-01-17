
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

void PackerMove::extendSequence(vector<Item> &sequence, mt19937 &rgen, const vector<vector<Item> > &all, int subseqId, int totalArea) {
  // Find the stacks in each sequence
  int remainingArea = totalArea;
  for (Item item : sequence)  remainingArea -= item.area();

  // TODO: prendre un item avant aussi bien qu'après
  set<int> stackSeen;
  vector<Item> candidates;
  for (size_t i = subseqId + 1; i < all.size(); ++i) {
    for (Item item : all[i]) {
      if (stackSeen.count(item.stack))
        continue;
      stackSeen.insert(item.stack);
      if (item.area() > remainingArea)
        continue;
      candidates.push_back(item);
    }
  }
  
  if (candidates.empty()) return;

  int pickedPos = uniform_int_distribution<int>(0, candidates.size() - 1)(rgen);
  int insertionPos = uniform_int_distribution<int>(0, sequence.size())(rgen);
  sequence.insert(sequence.begin() + insertionPos, candidates[pickedPos]);
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

Solution PackRowInsert::apply(mt19937& rgen) {
  // Select a random row to reoptimize
  vector<RowSolution> rows = extractRows(solution());
  int rowId = uniform_int_distribution<int>(0, rows.size() - 1)(rgen);
  RowSolution targetRow = rows[rowId];

  auto allRows = extractRowItems(solution());
  vector<Item> sequence = extractSequence(rows[rowId]);
  size_t initialSize = sequence.size();
  extendSequence(sequence, rgen, allRows, rowId, targetRow.area());

  if (sequence.size() == initialSize) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  RowPacker packer(sequence, params());
  RowSolution newSol = packer.run(targetRow, 0, problem().plateDefects()[plateIdOfRow(rowId)]);

  if (newSol.nItems() <= targetRow.nItems()) {
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

Solution PackCutInsert::apply(mt19937& rgen) {
  // Select a random cut to reoptimize
  vector<CutSolution> cuts = extractCuts(solution());
  int cutId = uniform_int_distribution<int>(0, cuts.size() - 1)(rgen);
  CutSolution targetCut = cuts[cutId];

  auto allCuts = extractCutItems(solution());
  vector<Item> sequence = extractSequence(cuts[cutId]);
  size_t initialSize = sequence.size();
  extendSequence(sequence, rgen, allCuts, cutId, targetCut.area());

  if (sequence.size() == initialSize) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  CutPacker packer(sequence, params());
  CutSolution newSol = packer.run(targetCut, 0, problem().plateDefects()[plateIdOfCut(cutId)]);

  if (newSol.nItems() <= targetCut.nItems()) {
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

Solution PackPlateInsert::apply(mt19937& rgen) {
  // Select a random plate to reoptimize
  vector<PlateSolution> plates = extractPlates(solution());
  int plateId = uniform_int_distribution<int>(0, plates.size() - 1)(rgen);
  PlateSolution targetPlate = plates[plateId];

  auto allPlates = extractPlateItems(solution());
  vector<Item> sequence = extractSequence(plates[plateId]);
  size_t initialSize = sequence.size();
  extendSequence(sequence, rgen, allPlates, plateId, targetPlate.area());

  if (sequence.size() == initialSize) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  PlatePacker packer(problem(), sequence, params());
  PlateSolution newSol = packer.run(plateId, 0);

  if (newSol.nItems() <= targetPlate.nItems()) {
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

