
#include "sequence_merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

Solution SequenceMerger::run(const Problem &problem, const std::pair<std::vector<Item>, std::vector<Item> > &sequences, SolverParams options) {
  SequenceMerger merger(problem, sequences, options);
  return merger.run();
}

SequenceMerger::SequenceMerger(const Problem &problem, const std::pair<std::vector<Item>, std::vector<Item> > &sequences, SolverParams options)
: problem_(problem)
, sequences_(sequences)
, options_(options) {
}

Solution SequenceMerger::run() {
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

    plates.emplace_back(options_, sequences_);
    plates.back().init(plateRegion, problem_.plateDefects()[i], starts);
    plates.back().buildFront();
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

