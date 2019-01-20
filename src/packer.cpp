
#include "packer.hpp"

#include <cassert>

using namespace std;

int Packer::firstValidVerticalCut(int minX, bool tightX) const {
  return firstValidVerticalCutFrom(region_.minX(), minX, tightX);
}

int Packer::firstValidHorizontalCut(int minY, bool tightY) const {
  return firstValidHorizontalCutFrom(region_.minY(), minY, tightY);
}

int Packer::firstValidVerticalCutFrom(int fromX, int minX, bool tightX) const {
  int minNonTight = tightX ? minX + Params::minWaste : minX;
  if (minX < fromX + Params::minXX) {
    minX = max(fromX + Params::minXX, minNonTight);
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
    // If a defect is at the border of the plate
    minX = min(minX, region_.maxX());
    if (!hasDefect)
      return minX;
    minX = max(minNonTight, minX);
  }
}

int Packer::firstValidHorizontalCutFrom(int fromY, int minY, bool tightY) const {
  int minNonTight = tightY ? minY + Params::minWaste : minY;
  if (minY < fromY + Params::minYY) {
    minY = max(fromY + Params::minYY, minNonTight);
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
    // If a defect is at the border of the plate
    minY = min(minY, region_.maxY());
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
    assert (region_.contains(defect));
  }
}

vector<int> Packer::extractFrontChanges(const vector<int> &front) {
  vector<int> changes;
  for (size_t i = 1; i < front.size(); ++i) {
    if (front[i] != front[i-1])
      changes.push_back(i);
  }
  return changes;
}

