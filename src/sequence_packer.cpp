
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

Solution SequencePacker::run(const Problem &problem, const std::vector<Item> &sequence) {
  SequencePacker packer(problem, sequence);
  packer.run();
  return packer.solution_;
}

SequencePacker::SequencePacker(const Problem &problem, const std::vector<Item> &sequence)
: problem_(problem)
, sequence_(sequence) {
  packedItems_ = 0;
}

void SequencePacker::run() {
  for (int i = 0; i < problem_.params().nPlates; ++i) {
    if (packedItems_ == (int) sequence_.size()) break;
    PlateSolution plate = PlatePacker::run(problem_, i, sequence_, packedItems_);
    packedItems_ += plate.nItems();
    solution_.plates.push_back(plate);
  }
}

