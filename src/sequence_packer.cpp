
#include "sequence_packer.hpp"
#include "plate_packer.hpp"

#include <cassert>
#include <algorithm>

/*
 * Dynamic programming on all cutting points
 *
 *
 * TODO:
 *  * prune infeasible cases
 *  * skip redundant cases (for example from solutions with whitespace)
 */

using namespace std;

Solution SequencePacker::run(const Problem &problem, const vector<Item> &sequence) {
  SequencePacker packer(problem, sequence);
  packer.run();
  return packer.solution_;
}

Solution SequencePacker::run(const Problem &problem, const vector<int> &sequence) {
  vector<Item> items;
  for (int id : sequence) {
    assert (id >= 0 && id < (int) problem.items().size());
    items.push_back(problem.items()[id]);
  }
  return run(problem, items);
}

SequencePacker::SequencePacker(const Problem &problem, const vector<Item> &sequence)
: problem_(problem)
, sequence_(sequence) {
  packedItems_ = 0;
}

void SequencePacker::run() {
  for (int i = 0; i < Params::nPlates; ++i) {
    if (packedItems_ == (int) sequence_.size()) break;
    PlateSolution plate = PlatePacker::run(problem_, i, sequence_, packedItems_);
    packedItems_ += plate.nItems();
    solution_.plates.push_back(plate);
  }
}

