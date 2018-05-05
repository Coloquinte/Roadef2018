
#include "problem.hpp"
#include "io_problem.hpp"

#include <map>
#include <algorithm>
#include <cassert>

using namespace std;

Problem::Problem(Params params, vector<Item> items, vector<Defect> defects)
: items_(items)
, defects_(defects)
, params_(params)
{
  buildSequences();
  buildPlates();
}

void Problem::buildSequences() {
  map<int, vector<Item> > stackToItems;
  for (Item i : items_) {
    stackToItems[i.stack].push_back(i);
  }
  for (auto p : stackToItems) {
    vector<Item> stack = p.second;
    sort(stack.begin(), stack.end(), [](Item a, Item b) { return a.sequence < b.sequence; });
    stackItems_.push_back(stack);
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

void Problem::checkConsistency() const {
  for (Item item : items_) {
    assert (item.width <= item.height);
  }
  // TODO: check IDs
  //  * use only consecutive IDs
  //  * convert only on read-write
}

Problem Problem::read(string nameItems, string nameDefects, string nameParams) {
  IOProblem io(nameItems, nameDefects, nameParams);
  return io.read();
}

void Problem::write(string nameItems, string nameDefects, string nameParams) const {
  IOProblem io(nameItems, nameDefects, nameParams);
  io.write(*this);
}

