
#include "solution.hpp"

#include <ostream>

using namespace std;

ItemSolution::ItemSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

RowSolution::RowSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

CutSolution::CutSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

PlateSolution::PlateSolution(int x, int y, int w, int h) {
  *(Rectangle*) this = Rectangle::FromDimensions(x, y, w, h);
}

int RowSolution::nItems() const {
  return items.size();
}

int RowSolution::maxUsedY() const {
  int maxUsed = minY();
  for (ItemSolution item : items) {
    maxUsed = max(item.maxY(), maxUsed);
  }
  return maxUsed;
}

int RowSolution::maxUsedX() const {
  if (items.empty())
    return minX();
  else
    return items.back().maxX();
}

int CutSolution::nItems() const {
  int cnt = 0;
  for (const RowSolution &row : rows)
    cnt += row.nItems();
  return cnt;
}

int CutSolution::maxUsedX() const {
  int maxUsed = minX();
  for (const RowSolution &row : rows) {
    maxUsed = max(row.maxUsedX(), maxUsed);
  }
  return maxUsed;
}

int PlateSolution::nItems() const {
  int cnt = 0;
  for (const CutSolution &cut : cuts)
    cnt += cut.nItems();
  return cnt;
}

int Solution::nItems() const {
  int cnt = 0;
  for (const PlateSolution &plate : plates)
    cnt += plate.nItems();
  return cnt;
}

void Solution::write(ostream &s) const {
  int plateId = 0;
  for (const PlateSolution &plate : plates) {
    s << "Plate #" << plateId++ << endl;
    int cutId = 0;
    for (const CutSolution &cut : plate.cuts) {
      s << "\tCut #" << cutId++ << " from " << cut.minX() << " to " << cut.maxX() << "  (" << cut.width() << "x" << cut.height() << ")" << endl;
      int rowId = 0;
      for (const RowSolution &row : cut.rows) {
        s << "\t\tRow #" << rowId++ << " from " << row.minY() << " to " << row.maxY() << "  (" << row.width() << "x" << row.height() << ")" << endl;
        for (const ItemSolution &item: row.items) {
          s << "\t\t\tItem #" << item.itemId << " from " << item.minX() << " to " << item.maxX() << "  (" << item.width() << "x" << item.height() << ")" << endl;
          //s << "\t\t\tItem #" << item.itemId << " at (" << item.minX() << ", " << item.maxX() << ")x(" << item.minY() << ", " << item.maxY() << ")" << endl;
        }
      }
    }
  }
  s << endl;
}

