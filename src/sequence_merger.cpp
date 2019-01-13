
#include "sequence_merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

using namespace std;

Solution SequenceMerger::run(const Problem &problem, const pair<vector<Item>, vector<Item> > &sequences, SolverParams options) {
  SequenceMerger merger(problem, sequences, options);
  return merger.run();
}

Solution SequenceMerger::run(const Problem &problem, const vector<Item> &sequence, SolverParams options) {
  pair<vector<Item>, vector<Item> > sequences;
  sequences.first = sequence;
  return run(problem, sequences, options);
}

SequenceMerger::SequenceMerger(const Problem &problem, const pair<vector<Item>, vector<Item> > &sequences, SolverParams options)
: problem_(problem)
, sequences_(sequences)
, options_(options) {
}

Solution SequenceMerger::run() {
  long long cutCalls = 0;
  long long rowCalls = 0;
  long long cutPrunedCalls = 0;
  long long rowPrunedCalls = 0;
  // Run plate by plate
  Rectangle plateRegion = Rectangle::FromCoordinates(0, 0, Params::widthPlates, Params::heightPlates);
  vector<PlateMerger> plates;
  for (int i = 0; i < Params::nPlates; ++i) {
    vector<pair<int, int> > starts;
    if (i == 0) {
      starts.emplace_back(0, 0);
    }
    else {
      starts = plates[i-1].getParetoFront(false);
    }
    assert (!starts.empty());

    // Check if finished
    if (starts.front().first  == (int) sequences_.first.size()
     && starts.front().second == (int) sequences_.second.size())
      break;

    if (options_.verbosity >= 4) {
      cout << "Plate " << i << " processed" << endl;
    }

    plates.emplace_back(options_, sequences_);
    plates.back().init(plateRegion, problem_.plateDefects()[i], starts);
    plates.back().buildFront();
    cutCalls   += plates.back().nCutCalls();
    rowCalls   += plates.back().nRowCalls();
    cutPrunedCalls   += plates.back().nPrunedCutCalls();
    rowPrunedCalls   += plates.back().nPrunedRowCalls();
  }
  if (options_.verbosity >= 3) {
    cout << plates.size() << " plate calls, "
         << cutCalls << " cut calls (" << (100.0 * cutPrunedCalls / (cutCalls + cutPrunedCalls)) << "% pruned), "
         << rowCalls << " row calls (" << (100.0 * rowPrunedCalls / (rowCalls + rowPrunedCalls)) << "% pruned) "
         << endl;
  }

  // Backtrack
  Solution solution;
  if (plates.empty())
    return solution;

  auto front = plates.back().getParetoFront(false);
  if (front.empty())
    return solution;

  pair<int, int> nPacked = front.front();
  for (auto it = plates.rbegin(); it != plates.rend(); ++it) {
    solution.plates.push_back(it->getSolution(nPacked, false));
    nPacked = it->getStarts(nPacked, false);
  }

  reverse(solution.plates.begin(), solution.plates.end());

  return solution;
}

