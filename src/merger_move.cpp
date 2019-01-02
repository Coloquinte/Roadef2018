
#include "merger_move.hpp"
#include "row_merger.hpp"
#include "cut_merger.hpp"
#include "plate_merger.hpp"
#include "sequence_merger.hpp"

#include <set>
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

pair<vector<Item>, vector<Item> > MergerMove::getRandomStacksSequences(mt19937 &rgen, const vector<Item> &subseq) {
  if (subseq.empty())
    return pair<vector<Item>, vector<Item> >();

  // Gather the stacks in the sequence
  set<int> stacks;
  for (Item item : subseq) stacks.insert(item.stack);

  // Split them in two (non-empty) groups
  vector<int> stackVec(stacks.begin(), stacks.end());
  shuffle(stackVec.begin(), stackVec.end(), rgen);
  assert (!stackVec.empty());
  int nbStacks = 1;
  if (stackVec.size() >= 2)
    nbStacks = uniform_int_distribution<int>(1, stackVec.size() - 1)(rgen);
  set<int> firstSeq(stackVec.begin(), stackVec.begin() + nbStacks);

  // Make two sequences based on those stacks
  pair<vector<Item>, vector<Item> > ret;
  for (Item item : subseq) {
    if (firstSeq.count(item.stack))
      ret.first.push_back(item);
    else
      ret.second.push_back(item);
  }

  return ret;
}

pair<vector<Item>, vector<Item> > MergerMove::getOneStackSequences(mt19937 &rgen, const vector<Item> &subseq) {
  if (subseq.empty())
    return pair<vector<Item>, vector<Item> >();

  // Gather the stacks in the sequence
  set<int> stacks;
  for (Item item : subseq) stacks.insert(item.stack);

  // Pick one and only one
  vector<int> stackVec(stacks.begin(), stacks.end());
  shuffle(stackVec.begin(), stackVec.end(), rgen);
  assert (!stackVec.empty());
  int selectedStack = stackVec.front();

  // Make two sequences based on those stacks
  pair<vector<Item>, vector<Item> > ret;
  for (Item item : subseq) {
    if (item.stack == selectedStack)
      ret.first.push_back(item);
    else
      ret.second.push_back(item);
  }

  return ret;
}

void MergerMove::extendMergeableSequences(pair<vector<Item>, vector<Item> > &sequences, mt19937 &rgen, const vector<vector<Item> > &all, int subseqId, int totalArea) {
  // Find the stacks in each sequence
  pair<set<int>, set<int> > stacksBySeq;
  for (Item item : sequences.first) stacksBySeq.first.insert(item.stack);
  for (Item item : sequences.second) stacksBySeq.second.insert(item.stack);

  // They must be disjoint
  for (int stack : stacksBySeq.first) assert(!stacksBySeq.second.count(stack));
  for (int stack : stacksBySeq.second) assert(!stacksBySeq.first.count(stack));

  int remainingArea = totalArea;
  for (Item item : sequences.first)  remainingArea -= item.area();
  for (Item item : sequences.second) remainingArea -= item.area();

  set<int> stackSeen;
  pair<vector<Item>, vector<Item> > candidates;
  vector<Item> anySeq;
  for (size_t i = subseqId + 1; i < all.size(); ++i) {
    for (Item item : all[i]) {
      if (stackSeen.count(item.stack))
        continue;
      stackSeen.insert(item.stack);
      if (item.area() > remainingArea)
        continue;
      if (stacksBySeq.first.count(item.stack)) {
        candidates.first.push_back(item);
      }
      else if (stacksBySeq.first.count(item.stack)) {
        candidates.second.push_back(item);
      }
      else {
        // An item that is not on a used stack can go on any sequence
        anySeq.push_back(item);
      }
    }
  }

  shuffle(anySeq.begin(), anySeq.end(), rgen);

  // Now extend both sequences by one
  if (!candidates.first.empty() && (anySeq.empty() || bernoulli_distribution()(rgen))) {
    shuffle(candidates.first.begin(), candidates.first.end(), rgen);
    sequences.first.push_back(candidates.first.back());
  }
  else if (!anySeq.empty()) {
    sequences.first.push_back(anySeq.back());
    anySeq.pop_back();
  }
  if (!candidates.second.empty() && (anySeq.empty() || bernoulli_distribution()(rgen))) {
    shuffle(candidates.second.begin(), candidates.second.end(), rgen);
    sequences.second.push_back(candidates.second.back());
  }
  else if (!anySeq.empty()) {
    sequences.second.push_back(anySeq.back());
    anySeq.pop_back();
  }
}

vector<pair<int, int> > MergerMove::getImprovingFront(const vector<pair<int, int> > &front, pair<int, int> best) {
  vector<pair<int, int> > betterFront;
  for (pair<int, int> elt : front) {
    if (elt.first < best.first || elt.second < best.second)
      continue;
    if (elt.first > best.first || elt.second > best.second)
      betterFront.push_back(elt);
  }
  return betterFront;
}

vector<Item> MergerMove::recreateFullSequence(const vector<Item> &newSubseq, const vector<vector<Item> > &all, int id) {
  set<int> seenIds;
  vector<Item> ret;
  for (int i = 0; i < id; ++i) {
    for (Item item : all[i]) {
      ret.push_back(item);
      //assert (!seenIds.count(item.id));
      seenIds.insert(item.id);
    }
  }
  for (Item item : newSubseq) {
    ret.push_back(item);
    //assert (!seenIds.count(item.id));
    seenIds.insert(item.id);
  }

  for (Item item : all[id]) {
    assert (seenIds.count(item.id));
  }

  for (int i = id + 1; i < (int) all.size(); ++i) {
    for (Item item : all[i]) {
      if (!seenIds.count(item.id))
        ret.push_back(item);
    }
  }

  return ret;
}

Solution MergeRow::apply(mt19937& rgen) {
  // Select a random row to reoptimize
  vector<RowSolution> rows = extractRows(solution());
  int rowId = uniform_int_distribution<int>(0, rows.size() - 1)(rgen);
  RowSolution targetRow = rows[rowId];

  // Extract two sequences from it
  vector<Item> subseq = extractSequence(targetRow);
  pair<vector<Item>, vector<Item> > sequences = getRandomStacksSequences(rgen, subseq);
  pair<int, int> initial(sequences.first.size(), sequences.second.size());

  auto allRows = extractRowItems(solution());
  extendMergeableSequences(sequences, rgen, allRows, rowId, targetRow.area());

  pair<int, int> possible(sequences.first.size(), sequences.second.size());
  if (possible == initial) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  // Merge the sequences optimally
  RowMerger merger(params(), sequences);
  merger.init(targetRow, problem().plateDefects()[plateIdOfRow(rowId)], make_pair(0, 0));
  merger.buildFront();

  // Get all solutions that are better than the original
  vector<pair<int, int> > paretoFront = merger.getParetoFront();
  vector<pair<int, int> > front = getImprovingFront(paretoFront, initial);
  if (front.empty()) {
    if (params().verbosity >= 4)
      cout << "No improvement found" << endl;
    return Solution();
  }

  // Pick one such solution
  shuffle(front.begin(), front.end(), rgen);
  pair<int, int> selected = front.back();

  // Run the whole algorithm with the new sequence if an improvement was found
  vector<Item> newSubseq = extractSequence(merger.getSolution(selected));
  vector<Item> newSeq = recreateFullSequence(newSubseq, allRows, rowId);
  checkSequenceValid(newSeq);
  return runSequence(newSeq);
}

Solution MergeCut::apply(mt19937& rgen) {
  // Select a random cut to reoptimize
  vector<CutSolution> cuts = extractCuts(solution());
  int cutId = uniform_int_distribution<int>(0, cuts.size() - 1)(rgen);
  CutSolution targetCut = cuts[cutId];

  // Extract two sequences from it
  vector<Item> subseq = extractSequence(targetCut);
  pair<vector<Item>, vector<Item> > sequences = getRandomStacksSequences(rgen, subseq);
  pair<int, int> initial(sequences.first.size(), sequences.second.size());

  auto allCuts = extractCutItems(solution());
  extendMergeableSequences(sequences, rgen, allCuts, cutId, targetCut.area());

  pair<int, int> possible(sequences.first.size(), sequences.second.size());
  if (possible == initial) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  // Merge the sequences optimally
  CutMerger merger(params(), sequences);
  merger.init(targetCut, problem().plateDefects()[plateIdOfCut(cutId)], make_pair(0, 0));
  merger.buildFront();

  // Get all solutions that are better than the original
  vector<pair<int, int> > paretoFront = merger.getParetoFront();
  vector<pair<int, int> > front = getImprovingFront(paretoFront, initial);
  if (front.empty()) {
    if (params().verbosity >= 4)
      cout << "No improvement found" << endl;
    return Solution();
  }

  // Pick one such solution
  shuffle(front.begin(), front.end(), rgen);
  pair<int, int> selected = front.back();

  // Run the whole algorithm with the new sequence if an improvement was found
  vector<Item> newSubseq = extractSequence(merger.getSolution(selected));
  vector<Item> newSeq = recreateFullSequence(newSubseq, allCuts, cutId);
  checkSequenceValid(newSeq);
  return runSequence(newSeq);
}

Solution MergePlate::apply(mt19937& rgen) {
  // Select a random plate to reoptimize
  vector<PlateSolution> plates = solution().plates;
  int plateId = uniform_int_distribution<int>(0, plates.size() - 1)(rgen);
  PlateSolution targetPlate = plates[plateId];

  // Extract two sequences from it
  vector<Item> subseq = extractSequence(targetPlate);
  pair<vector<Item>, vector<Item> > sequences = getRandomStacksSequences(rgen, subseq);
  pair<int, int> initial(sequences.first.size(), sequences.second.size());

  auto allPlates = extractPlateItems(solution());
  extendMergeableSequences(sequences, rgen, allPlates, plateId, targetPlate.area());

  pair<int, int> possible(sequences.first.size(), sequences.second.size());
  if (possible == initial) {
    if (params().verbosity >= 4)
      cout << "No new item found fitting the given area" << endl;
    return Solution();
  }

  // Merge the sequences optimally
  PlateMerger merger(params(), sequences);
  merger.init(targetPlate, problem().plateDefects()[plateId], make_pair(0, 0));
  merger.buildFront();

  // Get all solutions that are better than the original
  vector<pair<int, int> > paretoFront = merger.getParetoFront();
  vector<pair<int, int> > front = getImprovingFront(paretoFront, initial);
  if (front.empty()) {
    if (params().verbosity >= 4)
      cout << "No improvement found" << endl;
    return Solution();
  }

  // Pick one such solution
  shuffle(front.begin(), front.end(), rgen);
  pair<int, int> selected = front.back();

  // Run the whole algorithm with the new sequence if an improvement was found
  vector<Item> newSubseq = extractSequence(merger.getSolution(selected));
  vector<Item> newSeq = recreateFullSequence(newSubseq, allPlates, plateId);
  checkSequenceValid(newSeq);
  return runSequence(newSeq);
}

Solution MergeRandomStacks::apply(mt19937& rgen) {
  vector<Item> sequence = extractSequence(solution());
  pair<vector<Item>, vector<Item> > sequences = getRandomStacksSequences(rgen, sequence);
  return SequenceMerger::run(problem(), sequences, params());
}

Solution MergeOneStack::apply(mt19937& rgen) {
  vector<Item> sequence = extractSequence(solution());
  pair<vector<Item>, vector<Item> > sequences = getOneStackSequences(rgen, sequence);
  return SequenceMerger::run(problem(), sequences, params());
}




