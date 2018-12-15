
#include "merger.hpp"
#include "utils.hpp"

#include <cassert>
#include <algorithm>

using namespace std;

void Merger::init(Rectangle region, const std::vector<Defect> &defects, const std::vector<std::pair<int, int> > &starts, int coord) {
  region_ = region;
  defects_.clear();
  for (Defect d : defects) {
    if (d.intersects(region_))
      defects_.push_back(d);
  }
  front_.clear();
  for (std::pair<int, int> start : starts) {
    front_.emplace_back(coord, -1, start);
  }
}

void Merger::checkConsistency() const {
  for (std::size_t i = 0; i + 1 < front_.size(); ++i) {
    assert (front_[i].coord <= front_[i+1].coord);
  }
}
 


