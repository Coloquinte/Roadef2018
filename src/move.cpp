
#include "move.hpp"

#include "sequence_packer.hpp"
#include "solution_checker.hpp"
#include "ordering_heuristic.hpp"

#include <unordered_map>

using namespace std;

vector<Item> Move::extractSequence(const Problem &problem, const Solution &solution) const {
  vector<Item> sequence;
  for (const PlateSolution &plate: solution.plates) {
    for (const CutSolution &cut: plate.cuts) {
      for (const RowSolution &row: cut.rows) {
        for (ItemSolution item : row.items) {
          sequence.push_back(problem.items()[item.itemId]);
        }
      }
    }
  }
  return sequence;
}

bool Move::sequenceValid(const Problem &problem, const vector<Item> &sequence) const {
  unordered_map<int, int> itemPositions;
  int pos = 0;
  for (Item item : sequence) {
    itemPositions[item.id] = pos++;
  }

  for (const vector<Item> &sequence : problem.stackItems()) {
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

void Move::accept(const Problem &problem, Solution &solution, const Solution &incumbent) {
  int violations = SolutionChecker::nViolations(problem, incumbent);
  if (violations != 0)
    return;

  double mapped = SolutionChecker::evalPercentMapped(problem, incumbent);
  if (mapped < 99.9999 && mapped < SolutionChecker::evalPercentMapped(problem, solution))
    return;

  double density = SolutionChecker::evalPercentDensity(problem, incumbent);
  if (density <= SolutionChecker::evalPercentDensity(problem, solution))
    return;

  solution = incumbent;
}

void SwapMove::apply(const Problem &problem, Solution &solution, mt19937 &rgen) {
  vector<Item> sequence = this->extractSequence(problem, solution);
  if (sequence.size() < 2)
    return;

  uniform_int_distribution<int> dist(0, sequence.size()-1);
  int i0 = dist(rgen);
  int i1 = dist(rgen);
  if (i0 == i1)
    return;
  swap(sequence[i0], sequence[i1]);
  if (!sequenceValid(problem, sequence))
    return;

  Solution incumbent = SequencePacker::run(problem, sequence);
  accept(problem, solution, incumbent);
}

void InsertMove::apply(const Problem &problem, Solution &solution, mt19937 &rgen) {
  vector<Item> sequence = this->extractSequence(problem, solution);
  if (sequence.size() < 3)
    return;

  uniform_int_distribution<int> dist(0, sequence.size()-1);
  int pickedIndex = dist(rgen);
  Item picked = sequence[pickedIndex];
  sequence.erase(sequence.begin() + pickedIndex);

  int insertionPoint = dist(rgen);
  sequence.insert(sequence.begin() + insertionPoint, picked);

  if (!sequenceValid(problem, sequence))
    return;

  Solution incumbent = SequencePacker::run(problem, sequence);
  accept(problem, solution, incumbent);
}

void ShuffleMove::apply(const Problem &problem, Solution &solution, mt19937 &rgen) {
  vector<Item> sequence = OrderingHeuristic::orderShuffleStacks(problem, rgen);
  Solution incumbent = SequencePacker::run(problem, sequence);
  accept(problem, solution, incumbent);
}

void StackShuffleMove::apply(const Problem &problem, Solution &solution, mt19937 &rgen) {
  vector<Item> sequence = OrderingHeuristic::orderKeepStacks(problem, rgen);
  Solution incumbent = SequencePacker::run(problem, sequence);
  accept(problem, solution, incumbent);
}



