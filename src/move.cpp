
#include "move.hpp"

#include "sequence_packer.hpp"
#include "sequence_merger.hpp"
#include "solution_checker.hpp"
#include "ordering_heuristic.hpp"

#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <iostream>

using namespace std;

Move::Move()
: nViolation_(0)
, nImprovement_(0)
, nDegradation_(0)
, nPlateau_(0)
, nFailure_(0)
, nCommonPrefixPlates_ (0)
, nCommonSuffixPlates_(0)
, nDifferentPlates_(0)
{
}

vector<Item> Move::extractSequence(const Solution &solution) const {
  vector<Item> sequence;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (ItemSolution item : row.items) {
          sequence.push_back(problem().items()[item.itemId]);
        }
      }
    }
  }
  return sequence;
}

vector<Item> Move::extractSequence(const PlateSolution &plate) const {
  vector<Item> sequence;
  for (const CutSolution &cut: plate.cuts) {
    for (const RowSolution &row: cut.rows) {
      for (ItemSolution item : row.items) {
        sequence.push_back(problem().items()[item.itemId]);
      }
    }
  }
  return sequence;
}

vector<Item> Move::extractSequence(const CutSolution &cut) const {
  vector<Item> sequence;
  for (const RowSolution &row: cut.rows) {
    for (ItemSolution item : row.items) {
      sequence.push_back(problem().items()[item.itemId]);
    }
  }
  return sequence;
}

vector<Item> Move::extractSequence(const RowSolution &row) const {
  vector<Item> sequence;
  for (ItemSolution item : row.items) {
    sequence.push_back(problem().items()[item.itemId]);
  }
  return sequence;
}

vector<vector<Item> > Move::extractItemItems(const Solution &solution) const {
  vector<vector<Item> > items;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (ItemSolution item : row.items) {
          vector<Item> itemSeq = {problem().items()[item.itemId]};
          items.push_back(itemSeq);
        }
      }
    }
  }
  return items;
}

vector<vector<Item> > Move::extractRowItems(const Solution &solution) const {
  vector<vector<Item> > rows;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        rows.push_back(extractSequence(row));
      }
    }
  }
  return rows;
}

vector<vector<Item> > Move::extractCutItems(const Solution &solution) const {
  vector<vector<Item> > cuts;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      cuts.push_back(extractSequence(cut));
    }
  }
  return cuts;
}

vector<vector<Item> > Move::extractPlateItems(const Solution &solution) const {
  vector<vector<Item> > plates;
  for (const PlateSolution &plate: solution.plates) {
    plates.push_back(extractSequence(plate));
  }
  return plates;
}

vector<ItemSolution> Move::extractItems(const Solution &solution) const {
  vector<ItemSolution> items;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (ItemSolution item : row.items) {
          items.push_back(item);
        }
      }
    }
  }
  return items;
}

vector<RowSolution> Move::extractRows(const Solution &solution) const {
  vector<RowSolution> rows;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        rows.push_back(row);
      }
    }
  }
  return rows;
}

vector<CutSolution> Move::extractCuts(const Solution &solution) const {
  vector<CutSolution> cuts;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      cuts.push_back(cut);
    }
  }
  return cuts;
}

vector<PlateSolution> Move::extractPlates(const Solution &solution) const {
  vector<PlateSolution> plates;
  for (const PlateSolution &plate: solution.plates) {
    plates.push_back(plate);
  }
  return plates;
}

int Move::plateIdOfRow(int rowId) const {
  int id = 0;
  for (int i = 0; i < (int) solution().plates.size(); ++i) {
    const PlateSolution &plate = solution().plates[i];
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        if (rowId == id++)
          return i;
      }
    }
  }
  assert (false);
  return -1;
}

int Move::plateIdOfCut(int cutId) const {
  int id = 0;
  for (int i = 0; i < (int) solution().plates.size(); ++i) {
    const PlateSolution &plate = solution().plates[i];
    for (const CutSolution &cut: plate.cuts) {
      if (cutId == id++)
        return i;
    }
  }
  assert (false);
  return -1;
}

Solution Move::mergeRepairRun(const vector<vector<Item> > &sequence) {
  vector<Item> merged = mergeSequence(sequence);
  repairSequence(merged);
  return runSequence(merged);
}

Solution Move::runSequence(const vector<Item> &sequence) {
  if (!sequenceValid(sequence))
    return Solution();

  if (params().packWithMerger)
    return SequenceMerger::run(problem(), sequence, params());
  else
    return SequencePacker::run(problem(), sequence, params());
}

void randomInsert(vector<vector<Item> > &vec, mt19937 &rgen) {
  if (vec.size() <= 2)
    return;
  uniform_int_distribution<int> dist1(0, vec.size()-1);
  int pickedIndex = dist1(rgen);
  vector<Item> picked = vec[pickedIndex];
  vec.erase(vec.begin() + pickedIndex);

  uniform_int_distribution<int> dist2(0, vec.size()-2);
  int insertionPoint = dist2(rgen);
  if (insertionPoint >= pickedIndex)
    ++insertionPoint;
  vec.insert(vec.begin() + insertionPoint, picked);
}

void randomSwap(vector<vector<Item> > &vec, mt19937 &rgen) {
  if (vec.size() <= 2)
    return;
  uniform_int_distribution<int> dist1(0, vec.size()-1);
  int i0 = dist1(rgen);

  uniform_int_distribution<int> dist2(0, vec.size()-2);
  int i1 = dist2(rgen);
  if (i1 >= i0)
    ++i1;

  swap(vec[i0], vec[i1]);
}

void randomRangeSwap(vector<vector<Item> > &vec, mt19937 &rgen) {
  if (vec.size() <= 4)
    return;
  uniform_int_distribution<int> dist(0, vec.size()-1);
  vector<int> lims;
  lims.push_back(dist(rgen));
  lims.push_back(dist(rgen));
  lims.push_back(dist(rgen));
  lims.push_back(dist(rgen));
  sort(lims.begin(), lims.end());

  int b1 = lims[0];
  int e1 = lims[1];
  int b2 = lims[2];
  int e2 = lims[3];

  vector<vector<Item> > ret;
  for (int i = 0; i < b1; ++i)
    ret.push_back(vec[i]);
  for (int i = b2; i < e2; ++i)
    ret.push_back(vec[i]);
  for (int i = e1; i < b2; ++i)
    ret.push_back(vec[i]);
  for (int i = b1; i < e1; ++i)
    ret.push_back(vec[i]);
  for (int i = e2; i < (int) vec.size(); ++i)
    ret.push_back(vec[i]);

  swap(vec, ret);
}

void randomMirror(vector<vector<Item> > &vec, mt19937 &rgen, int maxWidth) {
  assert (maxWidth >= 3);
  if (vec.size() <= 3)
    return;

  uniform_int_distribution<int> widthDist(3, maxWidth);
  int width = min(widthDist(rgen), (int) vec.size());
  uniform_int_distribution<int> beginDist(0, vec.size()-width);
  int begin = beginDist(rgen);
  reverse(vec.begin() + begin, vec.begin() + begin + width);
}

void randomAdjacentSwap(vector<vector<Item> > &vec, mt19937 &rgen) {
  if (vec.size() < 2)
    return;
  uniform_int_distribution<int> dist(0, vec.size()-2);
  int i = dist(rgen);
  swap(vec[i], vec[i + 1]);
}

vector<Item> Move::mergeSequence(const vector<vector<Item> > &vecvec) {
  vector<Item> ret;
  for (const vector<Item> &vec : vecvec) {
    for (Item item : vec) {
      ret.push_back(item);
    }
  }
  return ret;
}

void Move::repairSequence(vector<Item> &sequence) const {
  // Reorder the items in each stack so they meet the precedence constraints
  vector<Item> result;

  // Create the stacks from the sequences in order to tolerate partial sequences
  vector<vector<Item> > stacks(problem().stackItems().size());
  for (Item item : sequence) {
    stacks[item.stack].push_back(item);
  }
  for (vector<Item> &stack : stacks) {
    sort(stack.begin(), stack.end(), [](Item a, Item b) { return a.sequence < b.sequence; });
  }

  vector<int> stackCounts(problem().stackItems().size(), 0);
  for (Item item : sequence) {
    int stack = item.stack;
    int index = stackCounts[stack]++;
    assert (index < (int) stacks[stack].size());
    result.push_back(stacks[stack][index]);
  }

  swap(sequence, result);
}

bool Move::sequenceValid(const vector<Item> &sequence) const {
  if (sequence.size() != problem().items().size())
    return false;

  unordered_map<int, int> itemPositions;
  int pos = 0;
  for (Item item : sequence) {
    itemPositions[item.id] = pos++;
  }

  for (const vector<Item> &sequence : problem().stackItems()) {
    for (unsigned i = 0; i + 1 < sequence.size(); ++i) {
      int ida = sequence[i].id;
      int idb = sequence[i+1].id;
      if (itemPositions.count(idb) == 0)
        continue;
      if (itemPositions.count(ida) == 0)
        return false;
      if (itemPositions[ida] > itemPositions[idb])
        return false;
    }
  }

  return true;
}

void Move::checkSequenceValid(const vector<Item> &sequence) const {
  assert (sequence.size() == problem().items().size());

  unordered_map<int, int> itemPositions;
  int pos = 0;
  for (Item item : sequence) {
    itemPositions[item.id] = pos++;
  }

  for (const vector<Item> &sequence : problem().stackItems()) {
    for (unsigned i = 0; i + 1 < sequence.size(); ++i) {
      int ida = sequence[i].id;
      int idb = sequence[i+1].id;
      if (itemPositions.count(idb) == 0)
        continue;
      assert (itemPositions.count(ida) != 0);
      assert (itemPositions[ida] <= itemPositions[idb]);
    }
  }
}

Solution Shuffle::apply(mt19937& rgen) {
  vector<Item> sequence;
  if (windowSize_ == 0) {
    sequence = OrderingHeuristic::orderShuffle(problem(), rgen, chunkSize_);
  } else {
    vector<Item> initial = extractSequence(solution());
    sequence = OrderingHeuristic::orderShuffle(problem(), rgen,
        initial, chunkSize_, windowSize_);
  }
  return runSequence(sequence);
}

string Shuffle::name() const {
  stringstream ss;
  if (windowSize_ <= 0)
    ss << "FullShuffle-" << chunkSize_;
  else
    ss << "PartialShuffle-" << chunkSize_ << "-" << windowSize_;
  return ss.str();
}

Solution ItemInsert::apply(mt19937& rgen) {
  vector<vector<Item> > items = extractItemItems(solution());
  randomInsert(items, rgen);
  return mergeRepairRun(items);
}

Solution RowInsert::apply(mt19937& rgen) {
  vector<vector<Item> > rows = extractRowItems(solution());
  randomInsert(rows, rgen);
  return mergeRepairRun(rows);
}

Solution CutInsert::apply(mt19937& rgen) {
  vector<vector<Item> > cuts = extractCutItems(solution());
  randomInsert(cuts, rgen);
  return mergeRepairRun(cuts);
}

Solution PlateInsert::apply(mt19937& rgen) {
  vector<vector<Item> > plates = extractPlateItems(solution());
  randomInsert(plates, rgen);
  return mergeRepairRun(plates);
}

Solution ItemSwap::apply(mt19937& rgen) {
  vector<vector<Item> > items = extractItemItems(solution());
  randomSwap(items, rgen);
  return mergeRepairRun(items);
}

Solution RowSwap::apply(mt19937& rgen) {
  vector<vector<Item> > rows = extractRowItems(solution());
  randomSwap(rows, rgen);
  return mergeRepairRun(rows);
}

Solution CutSwap::apply(mt19937& rgen) {
  vector<vector<Item> > cuts = extractCutItems(solution());
  randomSwap(cuts, rgen);
  return mergeRepairRun(cuts);
}

Solution PlateSwap::apply(mt19937& rgen) {
  vector<vector<Item> > plates = extractPlateItems(solution());
  randomSwap(plates, rgen);
  return mergeRepairRun(plates);
}

Solution RangeSwap::apply(mt19937& rgen) {
  vector<vector<Item> > items = extractItemItems(solution());
  randomRangeSwap(items, rgen);
  return mergeRepairRun(items);
}

Solution AdjacentItemSwap::apply(mt19937& rgen) {
  vector<vector<Item> > items = extractItemItems(solution());
  randomAdjacentSwap(items, rgen);
  return mergeRepairRun(items);
}

Solution AdjacentRowSwap::apply(mt19937& rgen) {
  vector<vector<Item> > rows = extractRowItems(solution());
  randomAdjacentSwap(rows, rgen);
  return mergeRepairRun(rows);
}

Solution AdjacentCutSwap::apply(mt19937& rgen) {
  vector<vector<Item> > cuts = extractCutItems(solution());
  randomAdjacentSwap(cuts, rgen);
  return mergeRepairRun(cuts);
}

Solution AdjacentPlateSwap::apply(mt19937& rgen) {
  vector<vector<Item> > plates = extractPlateItems(solution());
  randomAdjacentSwap(plates, rgen);
  return mergeRepairRun(plates);
}

Solution Mirror::apply(mt19937& rgen) {
  vector<vector<Item> > items = extractItemItems(solution());
  randomMirror(items, rgen, width_);
  return mergeRepairRun(items);
}

string Mirror::name() const {
  stringstream ss;
  ss << "Mirror-" << width_;
  return ss.str();
}

