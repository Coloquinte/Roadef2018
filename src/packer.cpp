
#include "packer.hpp"

#include <cassert>

using namespace std;

int Packer::firstValidVerticalCut(int minX, bool tightX) const {
  int minNonTight = tightX ? minX + Params::minWaste : minX;
  if (minX < region_.minX() + Params::minXX) {
    minX = max(region_.minX() + Params::minXX, minNonTight);
    tightX = false;
  }
  while (true) {
    bool hasDefect = false;
    for (Defect d : defects_) {
      if (d.intersectsVerticalLine(minX)) {
        minX = max(d.maxX() + 1, minX);
        hasDefect = true;
      }
    }
    assert (minX <= region_.maxX());
    if (!hasDefect)
      return minX;
    minX = max(minNonTight, minX);
  }
}

int Packer::firstValidHorizontalCut(int minY, bool tightY) const {
  int minNonTight = tightY ? minY + Params::minWaste : minY;
  if (minY < region_.minY() + Params::minYY) {
    minY = max(region_.minY() + Params::minYY, minNonTight);
    tightY = false;
  }
  while (true) {
    bool hasDefect = false;
    for (Defect d : defects_) {
      if (d.intersectsHorizontalLine(minY)) {
        minY = max(d.maxY() + 1, minY);
        hasDefect = true;
      }
    }
    assert (minY <= region_.maxY());
    if (!hasDefect)
      return minY;
    minY = max(minNonTight, minY);
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


