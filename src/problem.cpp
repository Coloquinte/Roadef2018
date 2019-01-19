
#include "problem.hpp"
#include "io_problem.hpp"

#include <map>
#include <algorithm>
#include <cassert>

using namespace std;

Problem::Problem(vector<Item> items, vector<Defect> defects)
: items_(items)
, defects_(defects)
{
  buildSequences();
  buildPlates();
  checkConsistency();
}

void Problem::buildSequences() {
  map<int, vector<Item> > stackToItems;
  for (Item i : items_) {
    stackToItems[i.stack].push_back(i);
  }
  for (auto p : stackToItems) {
    // TODO: make sure that the stacks match the ids in the items
    vector<Item> stack = p.second;
    sort(stack.begin(), stack.end(), [](Item a, Item b) { return a.sequence < b.sequence; });
    stackItems_.push_back(stack);
  }
}

void Problem::buildPlates() {
  plateDefects_.resize(Params::nPlates);
  for (Defect d : defects_) {
    if (d.plateId < 0 || d.plateId >= Params::nPlates)
      continue;
    plateDefects_[d.plateId].push_back(d);
  }
}

void Problem::checkConsistency() const {
  for (int i = 0; i < (int) items_.size(); ++i) {
    Item item = items_[i];
    assert (item.width <= item.height);
    assert (item.id == i);
    assert (item.stack >= 0);
    assert (item.stack < (int) stackItems_.size());
  }
}

Problem Problem::read(string nameItems, string nameDefects, bool permissive) {
  IOProblem io(nameItems, nameDefects, string());
  io.setPermissive(permissive);
  return io.read();
}

void Problem::write(string nameItems, string nameDefects, string nameParams) const {
  IOProblem io(nameItems, nameDefects, nameParams);
  io.write(*this);
}

