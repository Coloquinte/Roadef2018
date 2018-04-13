
#include "problem.hpp"
#include "io_problem.hpp"

#include <unordered_map>
#include <algorithm>

using namespace std;

Problem::Problem(Params params, std::vector<Item> items, std::vector<Defect> defects)
: items_(items)
, defects_(defects)
, params_(params)
{
  buildSequences();
  buildPlates();
}

void Problem::buildSequences() {
  unordered_map<int, vector<Item> > sequenceToItems;
  for (Item i : items_) {
    sequenceToItems[i.sequence].push_back(i);
  }
  for (auto p : sequenceToItems) {
    std::vector<Item> sequence = p.second;
    sort(sequence.begin(), sequence.end(), [](Item a, Item b) { return a.sequence < b.sequence; });
    sequenceItems_.push_back(sequence);
  }
}

void Problem::buildPlates() {
  plateDefects_.resize(params_.nPlates);
  for (Defect d : defects_) {
    if (d.plate_id < 0 || d.plate_id >= params_.nPlates)
      continue;
    plateDefects_[d.plate_id].push_back(d);
  }
}

Problem Problem::read(std::string prefix) {
  IOProblem io(prefix);
  return io.read();
}

void Problem::write(std::string prefix) const {
  IOProblem io(prefix);
  io.write(*this);
}

