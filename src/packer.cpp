
#include "packer.hpp"

#include <cassert>

using namespace std;

int Packer::lowestHorizontalCut(int minY, bool tightY) const {
  int cur = minY;
  while (true) {
    bool hasDefect = false;
    for (Defect d : defects_) {
      if (d.intersectsHorizontalLine(cur)) {
        cur = max(d.maxY() + 1, cur);
        hasDefect = true;
      }
    }
    assert (cur <= region_.maxY());
    if (!hasDefect)
      return cur;
    cur = max(tightY ? minY + Params::minWaste : minY, cur);
  }
}

int Packer::lowestVerticalCut(int minX, bool tightX) const {
  int cur = minX;
  while (true) {
    bool hasDefect = false;
    for (Defect d : defects_) {
      if (d.intersectsVerticalLine(cur)) {
        cur = max(d.maxX() + 1, cur);
        hasDefect = true;
      }
    }
    assert (cur <= region_.maxX());
    if (!hasDefect)
      return cur;
    cur = max(tightX ? minX + Params::minWaste : minX, cur);
  }
}

void Packer::checkConsistency() const {
  assert (region_.height() >= Params::minYY);
  assert (region_.width() >= Params::minXX);
  checkItems();
  checkDefects();
}

void Packer::checkItems() const {
  for (int i = 0; i < nItems(); ++i) {
    assert (sequence_[i].height >= sequence_[i].width);
  }
}

void Packer::checkDefects() const {
  for (Defect defect : defects_) {
    assert (region_.containsStrictly(defect));
  }
}


