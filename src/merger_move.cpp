
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

vector<Item> MergerMove::recreateFullSequence(const vector<Item> &newSubseq, const vector<Item> &all) {
  unordered_set<int> subseqIds;
  for (Item item : newSubseq) {
    subseqIds.insert(item.id);
  }

  vector<Item> ret;
  for (Item item : all) {
    if (subseqIds.count(item.id))
      break;
    ret.push_back(item);
  }
  for (Item item : newSubseq) {
    ret.push_back(item);
  }

  unordered_set<int> seenIds;
  for (Item item : ret) {
    seenIds.insert(item.id);
  }

  for (Item item : all) {
    if (!seenIds.count(item.id))
      ret.push_back(item);
  }

  return ret;
}
 
Solution MergeRow::apply(mt19937& rgen) {
  // Select a random row to reoptimize
  int plateId = uniform_int_distribution<int>(0, solution().plates.size() - 1)(rgen);
  RowSolution targetRow = pickRandomRow(solution(), plateId, rgen);

  // Extract two sequences
  vector<Item> subseq = extractSequence(targetRow);
  pair<vector<Item>, vector<Item> > sequences = getMergeableSequences(rgen, subseq);
  pair<int, int> initial(sequences.first.size(), sequences.second.size());
  vector<Item> all = extractSequence(solution());
  extendMergeableSequences(sequences, rgen, all);

  // Merge the sequences optimally
  RowMerger merger(params(), sequences);
  merger.init(targetRow, problem().plateDefects()[plateId], make_pair(0, 0));
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

  cout << "Found!" << endl;
  if (!front.empty()) {
    cout << "Non empty improving front against " << initial.first << " " << initial.second << endl;
    for (auto p : front) {
      cout << p.first << " " << p.second << endl;
    }
  }
  cout << "Plate: " << plateId << endl;
  cout << "Row: " << targetRow.minX() << " to " << targetRow.maxX() << ", " << targetRow.minY() << " to " << targetRow.maxY() << endl;
  cout << "Items: ";
  for (ItemSolution item : targetRow.items)
    cout << item.itemId << " ";
  cout << endl;

  // Run the whole algorithm with the new sequence if an improvement was found
  RowSolution newRow = merger.getSolution(selected);
  cout << "New: ";
  for (ItemSolution item : newRow.items)
    cout << item.itemId << " ";
  cout << endl;
  assert (newRow.nItems() == selected.first + selected.second);
  assert (newRow == targetRow);
  vector<Item> newSubseq = extractSequence(newRow);
  vector<Item> newSeq = recreateFullSequence(newSubseq, all);
  assert (sequenceValid(newSeq));
  solution().write("before.csv");
  Solution ret = runSequence(newSeq);
  ret.write("after.csv");
  cout << "Sequence: ";
  for (Item item : newSeq)
    cout << item.id << " ";
  cout << endl;
  exit(0);
  return ret;
}
 
