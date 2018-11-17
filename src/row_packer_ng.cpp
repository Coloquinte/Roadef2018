
#include "row_packer_ng.hpp"
#include "solution.hpp"
#include "utils.hpp"

#include <cassert>

using namespace std;

RowPackerNg::RowPackerNg(int maxSequenceSize)
{
  init(maxSequenceSize);
  items_ = nullptr;
  nItems_ = 0;
  defects_ = nullptr;
  nDefects_ = 0;
}

RowPackerNg::RowDescription RowPackerNg::fitNoDefectsSimple(Rectangle row, const Item *items, int nItems) {
  reset(row, items, nItems);
  return fitNoDefectsSimple();
}

RowSolution RowPackerNg::solNoDefectsSimple(Rectangle row, const Item *items, int nItems) {
  reset(row, items, nItems);
  return solNoDefectsSimple();
}

RowPackerNg::RowDescription RowPackerNg::fitNoDefectsSimple() {
  const int width = row_.width();
  const int height = row_.height();
  int left = width;
  RowDescription description;
  for (int i = 0; i < nItems_; ++i) {
    Item item = items_[i];;
    if (utils::fitsMinWaste(item.height, height)
     && utils::fitsMinWaste(item.width, left)) {
      left -= item.width;
      heights_[i] = item.height;
    }
    else if (utils::fitsMinWaste(item.width, height)
          && utils::fitsMinWaste(item.height, left)) {
      left -= item.height;
      heights_[i] = item.width;
    }
    else {
      break;
    }
    ++description.nItems;
  }
  description.maxUsedX = row_.maxX() - left;
  description.tightX = true;
  fillYData(description);
  return description;
}

RowSolution RowPackerNg::solNoDefectsSimple() {
  RowSolution solution(row_);
  int width = row_.width();
  int height = row_.height();
  int left = width;
  for (int i = 0; i < nItems_; ++i) {
    Item item = items_[i];;
    if (utils::fitsMinWaste(item.height, height)
     && utils::fitsMinWaste(item.width, left)) {
      Rectangle r = Rectangle::FromDimensions(row_.maxX() - left, row_.minY(), item.width, item.height);
      solution.items.emplace_back(r, item.id);
      left -= item.width;
    }
    else if (utils::fitsMinWaste(item.width, height)
          && utils::fitsMinWaste(item.height, left)) {
      Rectangle r = Rectangle::FromDimensions(row_.maxX() - left, row_.minY(), item.height, item.width);
      solution.items.emplace_back(r, item.id);
      left -= item.height;
    }
    else {
      break;
    }
  }
  checkSolution(solution);
  return solution;
}

void RowPackerNg::fillYData(RowDescription &description) const {
  int maxHeight = 0;
  for (int i = 0; i < description.nItems; ++i) {
    maxHeight = max(heights_[i], maxHeight);
  }
  description.maxUsedY = row_.minY() + maxHeight;
  description.tightY = true;
  for (int i = 0; i < description.nItems; ++i) {
    if (heights_[i] == maxHeight)
      continue;
    if (heights_[i] + Params::minWaste > maxHeight)
      description.tightY = false;
  }
  if (!description.tightY) description.maxUsedY += Params::minWaste;
}

void RowPackerNg::checkConsistency() const {
  checkItems();
  checkDefects();
}

void RowPackerNg::checkItems() const {
  assert (items_ != nullptr);
  assert (nItems_ <= maxSequenceSize_);
  for (int i = 0; i < nItems_; ++i) {
    assert (items_[i].height >= items_[i].width);
  }
}

void RowPackerNg::checkDefects() const {
  for (int i = 0; i < nDefects_; ++i) {
    assert (row_.contains(defects_[i]));
  }
}

void RowPackerNg::checkSolution(const RowSolution &row) {
  if (row.items.empty()) return;

  for (const ItemSolution &item : row.items) {
    assert (row.contains(item));
    assert (utils::fitsMinWaste(row.minY(), item.minY()));
    assert (utils::fitsMinWaste(item.maxY(), row.maxY()));
    assert (item.maxY() == row.maxY() || item.minY() == row.minY());
    for (int j = 0; j < nDefects_; ++j) {
      Defect defect = defects_[j];
      assert (!defect.intersectsVerticalLine(item.minX()));
      assert (!defect.intersectsVerticalLine(item.maxX()));
    }
  }

  assert (utils::fitsMinWaste(row.minX(), row.items.front().minX()));
  assert (utils::fitsMinWaste(row.items.back().maxX(), row.maxX()));

  for (int i = 0; i+1 < (int) row.items.size(); ++i) {
    ItemSolution item1 = row.items[i];
    ItemSolution item2 = row.items[i+1];
    assert (item1.maxX() >= item2.minX());
    assert (utils::fitsMinWaste(item1.maxX(), item2.minX()));
  }
}

void RowPackerNg::checkEquivalent(const RowDescription &description, const RowSolution &solution) {
  assert (description.nItems == (int) solution.items.size());

  if (solution.items.empty()) {
    assert (description.tightX && description.tightY
        && description.maxUsedX == solution.minX()
        && description.maxUsedY == solution.minY());
    return;
  }

  assert (description.maxUsedX == solution.items.back().maxX());

  int maxUsedY = solution.minY();
  for (ItemSolution item : solution.items) {
    maxUsedY = max(maxUsedY, item.maxY());
  }

  // Do not check tightness on X because there are corner cases when defects are present
  bool tightY = true;
  for (ItemSolution item : solution.items) {
    if (item.maxY() == maxUsedY)
      continue;
    if (item.maxY() + Params::minWaste > maxUsedY)
      tightY = false;
  }
  if (!tightY) maxUsedY += Params::minWaste;

  assert (description.maxUsedY == maxUsedY);
  assert (description.tightY == tightY);
}

void RowPackerNg::init(int maxSequenceSize) {
  this->maxSequenceSize_ = maxSequenceSize;
  widths_.reset(new int[maxSequenceSize]);
  heights_.reset(new int[maxSequenceSize]);
  placements_.reset(new int[maxSequenceSize]);
}

void RowPackerNg::reset(Rectangle row, const Item *items, int nItems) {
  row_ = row;
  items_ = items;
  nItems_ = nItems;
  defects_ = nullptr;
  nDefects_ = 0;
}

void RowPackerNg::reset(Rectangle row, const Item *items, int nItems, const Defect *defects, int nDefects) {
  row_ = row;
  items_ = items;
  nItems_ = nItems;
  defects_ = defects;
  nDefects_ = nDefects;
}

