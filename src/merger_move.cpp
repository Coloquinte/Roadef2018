
#include "merger_move.hpp"
#include "row_merger.hpp"

#include <unordered_set>

using namespace std;

pair<vector<Item>, vector<Item> > MergerMove::getMergeableSequences(std::mt19937 &rgen, const vector<Item> &subseq) {
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

void MergerMove::extendMergeableSequences(std::pair<std::vector<Item>, std::vector<Item> > &sequences, std::mt19937 &rgen, const std::vector<Item> &all) {
  pair<unordered_set<int>, unordered_set<int> > stacksBySeq;
  for (Item item : sequences.first) stacksBySeq.first.insert(item.stack);
  for (Item item : sequences.second) stacksBySeq.second.insert(item.stack);

  // Find where the subsequence starts
  size_t seqBegin = 0;
  for (size_t i = 0; i < all.size(); ++i) {
    if (!sequences.first.empty() && all[i].id == sequences.first.back().id)
      seqBegin = i + 1;
    if (!sequences.second.empty() && all[i].id == sequences.second.back().id)
      seqBegin = i + 1;
  }

  // Try to push one item on each sequence
  for (size_t i = seqBegin; i < all.size(); ++i) {
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
 
Solution MergeRow::apply(std::mt19937& rgen) {
  // Select a random row to reoptimize
  // TODO
  int plateId;
  RowSolution targetRow;

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
  vector<pair<int, int> > front = getImprovingFront(merger.getParetoFront(), initial);
  if (front.empty()) return Solution();

  // Run the whole algorithm with the new sequence if an improvement was found
  // TODO
  return Solution();
}
 
