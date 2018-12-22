
#include "merger_move.hpp"
#include "row_merger.hpp"

#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

pair<vector<Item>, vector<Item> > MergerMove::getMergeableSequences(mt19937 &rgen, const vector<Item> &subseq) {
  // Gather the stacks in the sequence
  unordered_set<int> stacks;
  for (Item item : subseq) stacks.insert(item.stack);

  // Split them in two groups
  pair<unordered_set<int>, unordered_set<int> > stacksBySeq;
  for (int stack : stacks) {
    if (bernoulli_distribution()(rgen))
      stacksBySeq.first.insert(stack);
    else
      stacksBySeq.second.insert(stack);
  }

  // Make two sequences based on those stacks
  pair<vector<Item>, vector<Item> > ret;
  for (Item item : subseq) {
    if (stacksBySeq.first.count(item.stack))
      ret.first.push_back(item);
    else
      ret.second.push_back(item);
  }

  return ret; 
}

void MergerMove::extendMergeableSequences(pair<vector<Item>, vector<Item> > &sequences, mt19937 &rgen, const vector<Item> &all) {
  // Find the stacks in each sequence
  pair<unordered_set<int>, unordered_set<int> > stacksBySeq;
  for (Item item : sequences.first) stacksBySeq.first.insert(item.stack);
  for (Item item : sequences.second) stacksBySeq.second.insert(item.stack);

  // They must be disjoint
  for (int stack : stacksBySeq.first) assert(!stacksBySeq.second.count(stack));
  for (int stack : stacksBySeq.second) assert(!stacksBySeq.first.count(stack));

  // Find where the subsequence starts
  size_t seqEnd = 0;
  for (size_t i = 0; i < all.size(); ++i) {
    if (!sequences.first.empty() && all[i].id == sequences.first.back().id)
      seqEnd = i + 1;
    if (!sequences.second.empty() && all[i].id == sequences.second.back().id)
      seqEnd = i + 1;
  }

  unordered_set<int> stackSeen;
  pair<vector<Item>, vector<Item> > candidates;
  vector<Item> anySeq;
  for (size_t i = seqEnd; i < all.size(); ++i) {
    Item item = all[i];
    if (stackSeen.count(item.stack))
      continue;
    stackSeen.insert(item.stack);
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
  unordered_set<int> seenIds;
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
  pair<vector<Item>, vector<Item> > sequences = getMergeableSequences(rgen, subseq);
  pair<int, int> initial(sequences.first.size(), sequences.second.size());
  vector<Item> all = extractSequence(solution());
  extendMergeableSequences(sequences, rgen, all);

  auto allRows = extractRowItems(solution());
  unordered_set<int> beforeIds;
  for (int i = 0; i < rowId; ++i) {
    for (Item item : allRows[i])
      beforeIds.insert(item.id);
  }
  for (Item item : sequences.first) {
    assert(!beforeIds.count(item.id));
  }
  for (Item item : sequences.second) {
    assert(!beforeIds.count(item.id));
  }

  // Merge the sequences optimally
  RowMerger merger(params(), sequences);
  merger.init(targetRow, problem().plateDefects()[plateIdOfRow(rowId)], make_pair(0, 0));
  merger.buildFront();

  // Get all solutions that are better than the original
  vector<pair<int, int> > paretoFront = merger.getParetoFront();
  vector<pair<int, int> > front = getImprovingFront(paretoFront, initial);
  if (front.empty()) return Solution();

  // Pick one such solution
  shuffle(front.begin(), front.end(), rgen);
  pair<int, int> selected = front.back();
  assert (selected.first >= initial.first && selected.second >= initial.second);
  assert (selected.first > initial.first || selected.second > initial.second);

  // Run the whole algorithm with the new sequence if an improvement was found
  RowSolution newRow = merger.getSolution(selected);
  assert (newRow.nItems() == selected.first + selected.second);
  assert (newRow == targetRow);

  vector<Item> newSubseq = extractSequence(newRow);
  vector<Item> newSeq = recreateFullSequence(newSubseq, extractRowItems(solution()), rowId);
  checkSequenceValid(newSeq);
  return runSequence(newSeq);
}
 
