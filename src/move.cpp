
#include "move.hpp"

#include "sequence_packer.hpp"
#include "solution_checker.hpp"
#include "ordering_heuristic.hpp"

#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <iostream>

using namespace std;

Move::Move()
: nCall_(0)
, nViolation_(0)
, nImprovement_(0)
, nDegradation_(0)
, nPlateau_(0) {
}

Move::Status Move::run() {
  ++nCall_;
  for (int i = 0; i < RETRY; ++i) {
    Status status = apply();
    updateStats(status);
    if (status != Status::Failure)
      return status;
  }
  return Status::Failure;
}

void Move::updateStats(Status status) {
  switch (status) {
    case Status::Improvement:
      ++nImprovement_;
      break;
    case Status::Degradation:
      ++nDegradation_;
      break;
    case Status::Plateau:
      ++nPlateau_;
      break;
    case Status::Violation:
      ++nViolation_;
      break;
    case Status::Failure:
      break;
  }
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

vector<vector<Item> > Move::extractRows(const Solution &solution) const {
  vector<vector<Item> > rows;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        vector<Item> rowSeq;
        for (ItemSolution item : row.items) {
          rowSeq.push_back(problem().items()[item.itemId]);
        }
        rows.push_back(rowSeq);
      }
    }
  }
  return rows;
}

vector<vector<Item> > Move::extractCuts(const Solution &solution) const {
  vector<vector<Item> > cuts;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      vector<Item> cutSeq;
      for (const RowSolution &row: cut.rows) {
        for (ItemSolution item : row.items) {
          cutSeq.push_back(problem().items()[item.itemId]);
        }
      }
      cuts.push_back(cutSeq);
    }
  }
  return cuts;
}

vector<vector<Item> > Move::extractPlates(const Solution &solution) const {
  vector<vector<Item> > plates;
  for (const PlateSolution &plate: solution.plates) {
    vector<Item> plateSeq;
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (ItemSolution item : row.items) {
          plateSeq.push_back(problem().items()[item.itemId]);
        }
      }
    }
    plates.push_back(plateSeq);
  }
  return plates;
}

Move::Status Move::runSequence(const vector<Item> &sequence) {
  if (!sequenceValid(sequence))
    return Status::Failure;

  Solution incumbent = SequencePacker::run(problem(), sequence);
  return accept(incumbent);
}

template<typename T>
void randomInsert(vector<T> &vec, mt19937 &rgen) {
  if (vec.size() <= 2)
    return;
  uniform_int_distribution<int> dist1(0, vec.size()-1);
  int pickedIndex = dist1(rgen);
  T picked = vec[pickedIndex];
  vec.erase(vec.begin() + pickedIndex);

  uniform_int_distribution<int> dist2(0, vec.size()-2);
  int insertionPoint = dist2(rgen);
  if (insertionPoint >= pickedIndex)
    ++insertionPoint;
  vec.insert(vec.begin() + insertionPoint, picked);
}

template<typename T>
void randomSwap(vector<T> &vec, mt19937 &rgen) {
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

template<typename T>
void randomMirror(vector<T> &vec, mt19937 &rgen, int maxWidth) {
  assert (maxWidth >= 3);
  if (vec.size() <= 3)
    return;

  uniform_int_distribution<int> widthDist(3, maxWidth);
  int width = min(widthDist(rgen), (int) vec.size());
  uniform_int_distribution<int> beginDist(0, vec.size()-width);
  int begin = beginDist(rgen);
  reverse(vec.begin() + begin, vec.begin() + begin + width);
}

template<typename T>
void randomAdjacentSwap(vector<T> &vec, mt19937 &rgen) {
  if (vec.size() < 2)
    return;
  uniform_int_distribution<int> dist(0, vec.size()-2);
  int i = dist(rgen);
  swap(vec[i], vec[i + 1]);
}

vector<Item> merge(const vector<vector<Item> > &vecvec) {
  vector<Item> ret;
  for (const vector<Item> &vec : vecvec) {
    for (Item item : vec) {
      ret.push_back(item);
    }
  }
  return ret;
}

bool Move::sequenceValid(const vector<Item> &sequence) const {
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

Move::Status Move::accept(const Solution &incumbent) {
  int violations = SolutionChecker::nViolations(problem(), incumbent);
  if (violations != 0) {
    if (solver_->params_.failOnViolation) {
      incumbent.report();
      SolutionChecker::report(problem(), incumbent);
      exit(1);
    }
    return Status::Violation;
  }

  double mapped = SolutionChecker::evalPercentMapped(problem(), incumbent);
  double prevMapped = SolutionChecker::evalPercentMapped(problem(), solution());
  if (mapped > prevMapped) {
    solution() = incumbent;
    return Status::Improvement;
  }
  if (mapped < prevMapped) {
    return Status::Degradation;
  }

  double density = SolutionChecker::evalPercentDensity(problem(), incumbent);
  double prevDensity = SolutionChecker::evalPercentDensity(problem(), solution());
  if (density > prevDensity) {
    solution() = incumbent;
    return Status::Improvement;
  }
  if (density < prevDensity) {
    return Status::Degradation;
  }

  solution() = incumbent;
  return Status::Plateau;
}

Move::Status Shuffle::apply() {
  vector<Item> sequence;
  if (windowSize_ == 0) {
    sequence = OrderingHeuristic::orderShuffle(problem(), rgen(), chunkSize_);
  } else {
    vector<Item> initial = extractSequence(solution());
    sequence = OrderingHeuristic::orderShuffle(problem(), rgen(),
        initial, chunkSize_, windowSize_);
  }
  assert (sequenceValid(sequence));
  Solution incumbent = SequencePacker::run(problem(), sequence);
  return accept(incumbent);
}

string Shuffle::name() const {
  stringstream ss;
  if (windowSize_ <= 0)
    ss << "FullShuffle-" << chunkSize_;
  else
    ss << "PartialShuffle-" << chunkSize_ << "-" << windowSize_;
  return ss.str();
}

Move::Status ItemInsert::apply() {
  vector<Item> sequence = extractSequence(solution());
  randomInsert(sequence, rgen());
  return runSequence(sequence);
}

Move::Status RowInsert::apply() {
  vector<vector<Item> > rows = extractRows(solution());
  randomInsert(rows, rgen());
  vector<Item> sequence = merge(rows);
  return runSequence(sequence);
}

Move::Status CutInsert::apply() {
  vector<vector<Item> > cuts = extractCuts(solution());
  randomInsert(cuts, rgen());
  vector<Item> sequence = merge(cuts);
  return runSequence(sequence);
}

Move::Status PlateInsert::apply() {
  vector<vector<Item> > plates = extractPlates(solution());
  randomInsert(plates, rgen());
  vector<Item> sequence = merge(plates);
  return runSequence(sequence);
}

Move::Status ItemSwap::apply() {
  vector<Item> sequence = extractSequence(solution());
  randomSwap(sequence, rgen());
  return runSequence(sequence);
}

Move::Status RowSwap::apply() {
  vector<vector<Item> > rows = extractRows(solution());
  randomSwap(rows, rgen());
  vector<Item> sequence = merge(rows);
  return runSequence(sequence);
}

Move::Status CutSwap::apply() {
  vector<vector<Item> > cuts = extractCuts(solution());
  randomSwap(cuts, rgen());
  vector<Item> sequence = merge(cuts);
  return runSequence(sequence);
}

Move::Status PlateSwap::apply() {
  vector<vector<Item> > plates = extractPlates(solution());
  randomSwap(plates, rgen());
  vector<Item> sequence = merge(plates);
  return runSequence(sequence);
}

Move::Status AdjacentItemSwap::apply() {
  vector<Item> sequence = extractSequence(solution());
  randomAdjacentSwap(sequence, rgen());
  return runSequence(sequence);
}

Move::Status AdjacentRowSwap::apply() {
  vector<vector<Item> > rows = extractRows(solution());
  randomAdjacentSwap(rows, rgen());
  vector<Item> sequence = merge(rows);
  return runSequence(sequence);
}

Move::Status AdjacentCutSwap::apply() {
  vector<vector<Item> > cuts = extractCuts(solution());
  randomAdjacentSwap(cuts, rgen());
  vector<Item> sequence = merge(cuts);
  return runSequence(sequence);
}

Move::Status AdjacentPlateSwap::apply() {
  vector<vector<Item> > plates = extractPlates(solution());
  randomAdjacentSwap(plates, rgen());
  vector<Item> sequence = merge(plates);
  return runSequence(sequence);
}

Move::Status Mirror::apply() {
  vector<Item> sequence = extractSequence(solution());
  randomMirror(sequence, rgen(), width_);
  return runSequence(sequence);
}

string Mirror::name() const {
  stringstream ss;
  ss << "Mirror-" << width_;
  return ss.str();
}




